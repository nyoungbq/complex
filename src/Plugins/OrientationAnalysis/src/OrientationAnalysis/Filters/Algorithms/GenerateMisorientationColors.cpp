#include "GenerateMisorientationColors.hpp"

#include "simplnx/Common/Constants.hpp"
#include "simplnx/Common/RgbColor.hpp"
#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataStore.hpp"
#include "simplnx/Utilities/Math/MatrixMath.hpp"
#include "simplnx/Utilities/ParallelDataAlgorithm.hpp"

#include "EbsdLib/Core/OrientationTransformation.hpp"
#include "EbsdLib/LaueOps/LaueOps.h"

using namespace nx::core;

using FloatVec3Type = std::vector<float>;

namespace
{

/**
 * @brief The GenerateMisorientationColorsImpl class implements a threaded algorithm that computes the Misorientation
 * colors for each element in a geometry
 */
template <bool UsingEulerAngles = true>
class GenerateMisorientationColorsImpl
{
public:
  GenerateMisorientationColorsImpl(GenerateMisorientationColors* filter, const std::vector<float32> referenceAxis, Float32Array& cellOrientations, Int32Array& phases, UInt32Array& crystalStructures,
                                   int32_t numPhases, const IDataArray* goodVoxels, UInt8Array& colors)
  : m_Filter(filter)
  , m_ReferenceAxis(referenceAxis)
  , m_CellOrientations(cellOrientations.getDataStoreRef())
  , m_CellPhases(phases.getDataStoreRef())
  , m_CrystalStructures(crystalStructures.getDataStoreRef())
  , m_NumPhases(numPhases)
  , m_GoodVoxels(goodVoxels)
  , m_CellMisorientationColors(colors.getDataStoreRef())
  {
  }

  virtual ~GenerateMisorientationColorsImpl() = default;

  template <typename T>
  void convert(size_t start, size_t end) const
  {
    using MaskArrayType = DataArray<T>;
    const MaskArrayType* maskArray = nullptr;
    if(nullptr != m_GoodVoxels)
    {
      maskArray = dynamic_cast<const MaskArrayType*>(m_GoodVoxels);
    }

    std::vector<LaueOps::Pointer> ops = LaueOps::GetAllOrientationOps();
    QuatD q2 = OrientationTransformation::ax2qu<std::vector<float32>, QuatD>(m_ReferenceAxis);
    std::array<double, 3> dEuler = {0.0, 0.0, 0.0};
    Rgba argb = 0x00000000;
    int32_t phase = 0;
    bool calcMisorientation = false;
    for(size_t i = start; i < end; i++)
    {
      phase = m_CellPhases[i];
      m_CellMisorientationColors.setValue(3 * i, 0);
      m_CellMisorientationColors.setValue(3 * i + 1, 0);
      m_CellMisorientationColors.setValue(3 * i + 2, 0);

      // Make sure we are using a valid Euler Angles with valid crystal symmetry
      calcMisorientation = true;
      if(nullptr != maskArray)
      {
        calcMisorientation = (*maskArray)[i];
      }
      // Sanity check the phase data to make sure we do not walk off the end of the array
      if(phase >= m_NumPhases)
      {
        m_Filter->incrementPhaseWarningCount();
      }

      if(phase < m_NumPhases && calcMisorientation && m_CrystalStructures[phase] < EbsdLib::CrystalStructure::LaueGroupEnd)
      {
        QuatD q1;
        if constexpr(UsingEulerAngles)
        {
          q1 = OrientationTransformation::eu2qu<std::vector<float32>, QuatD>(std::vector<float32>{m_CellOrientations[3 * i + 0], m_CellOrientations[3 * i + 1], m_CellOrientations[3 * i + 2]});
        }
        else
        {
          q1 = QuatD(m_CellOrientations[4 * i + 0], m_CellOrientations[4 * i + 1], m_CellOrientations[4 * i + 2], m_CellOrientations[4 * i + 3]);
        }

        OrientationD axisAngle = ops[m_CrystalStructures[phase]]->generateMisorientationColor(q1, q2);

        m_CellMisorientationColors.setValue(3 * i, axisAngle[0] * (axisAngle[3] * Constants::k_180OverPiD));
        m_CellMisorientationColors.setValue(3 * i + 1, axisAngle[1] * (axisAngle[3] * Constants::k_180OverPiD));
        m_CellMisorientationColors.setValue(3 * i + 2, axisAngle[0] * (axisAngle[3] * Constants::k_180OverPiD));
      }
    }
  }

  void run(size_t start, size_t end) const
  {
    if(m_GoodVoxels != nullptr)
    {
      if(m_GoodVoxels->getDataType() == DataType::boolean)
      {
        convert<bool>(start, end);
      }
      else if(m_GoodVoxels->getDataType() == DataType::uint8)
      {
        convert<uint8>(start, end);
      }
    }
    else
    {
      convert<bool>(start, end);
    }
  }

  void operator()(const Range& range) const
  {
    run(range.min(), range.max());
  }

private:
  GenerateMisorientationColors* m_Filter = nullptr;
  const std::vector<float32> m_ReferenceAxis;
  Float32AbstractDataStore& m_CellOrientations;
  Int32AbstractDataStore& m_CellPhases;
  UInt32AbstractDataStore& m_CrystalStructures;
  int32_t m_NumPhases = 0;
  const IDataArray* m_GoodVoxels = nullptr;
  UInt8AbstractDataStore& m_CellMisorientationColors;
};
} // namespace

// -----------------------------------------------------------------------------
GenerateMisorientationColors::GenerateMisorientationColors(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                                           GenerateMisorientationColorsInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
{
}

// -----------------------------------------------------------------------------
GenerateMisorientationColors::~GenerateMisorientationColors() noexcept = default;

// -----------------------------------------------------------------------------
Result<> GenerateMisorientationColors::operator()()
{
  std::vector<LaueOps::Pointer> orientationOps = LaueOps::GetAllOrientationOps();

  auto& cellOrientationArray = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->cellOrientationsArrayPath);
  auto& phases = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->cellPhasesArrayPath);

  auto& crystalStructures = m_DataStructure.getDataRefAs<UInt32Array>(m_InputValues->crystalStructuresArrayPath);

  auto& MisorientationColors = m_DataStructure.getDataRefAs<UInt8Array>(m_InputValues->cellMisorientationColorsArrayPath);

  m_PhaseWarningCount = 0;
  size_t totalPoints = cellOrientationArray.getNumberOfTuples();

  auto numPhases = static_cast<int32_t>(crystalStructures.getNumberOfTuples());

  typename IParallelAlgorithm::AlgorithmArrays algArrays;
  algArrays.push_back(&cellOrientationArray);
  algArrays.push_back(&phases);
  algArrays.push_back(&crystalStructures);
  algArrays.push_back(&MisorientationColors);

  IDataArray* goodVoxelsArray = nullptr;
  if(m_InputValues->useGoodVoxels)
  {
    goodVoxelsArray = m_DataStructure.getDataAs<IDataArray>(m_InputValues->goodVoxelsArrayPath);
    algArrays.push_back(goodVoxelsArray);
  }

  // Allow data-based parallelization
  ParallelDataAlgorithm dataAlg;
  dataAlg.setRange(0, totalPoints);
  dataAlg.requireArraysInMemory(algArrays);

  if(m_InputValues->useEulers)
  {
    dataAlg.execute(::GenerateMisorientationColorsImpl<true>(this, m_InputValues->referenceAxis, cellOrientationArray, phases, crystalStructures, numPhases, goodVoxelsArray, MisorientationColors));
  }
  if(!m_InputValues->useEulers)
  {
    dataAlg.execute(::GenerateMisorientationColorsImpl<false>(this, m_InputValues->referenceAxis, cellOrientationArray, phases, crystalStructures, numPhases, goodVoxelsArray, MisorientationColors));
  }

  if(m_PhaseWarningCount > 0)
  {
    std::string message = fmt::format("The Ensemble Phase information only references {} phase(s) but {} cell(s) had a phase value greater than {}. This indicates a problem with the input cell phase "
                                      "data. DREAM.3D will give INCORRECT RESULTS.",
                                      (numPhases - 1), m_PhaseWarningCount, (numPhases - 1));
    return MakeErrorResult(-48000, message);
  }

  return {};
}

// -----------------------------------------------------------------------------
void GenerateMisorientationColors::incrementPhaseWarningCount()
{
  ++m_PhaseWarningCount;
}
