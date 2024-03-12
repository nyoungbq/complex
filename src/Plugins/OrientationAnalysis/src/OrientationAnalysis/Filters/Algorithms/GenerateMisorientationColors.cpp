#include "GenerateMisorientationColors.hpp"

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
template<bool UsingEulerAngles = true>
class GenerateMisorientationColorsImpl
{
public:
  GenerateMisorientationColorsImpl(GenerateMisorientationColors* filter, FloatVec3Type referenceAxis, Float32Array& cellOrientations, Int32Array& phases, UInt32Array& crystalStructures,
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
    std::array<double, 3> refDir = {m_ReferenceDir[0], m_ReferenceDir[1], m_ReferenceDir[2]};
    QuatF q2 = OrientationTransformation::ax2qu<std::vector<float32>, QuatF>(m_ReferenceAxis);
    std::array<double, 3> dEuler = {0.0, 0.0, 0.0};
    Rgba argb = 0x00000000;
    int32_t phase = 0;
    bool calcMisorientation = false;
    size_t index = 0;
    for(size_t i = start; i < end; i++)
    {
      phase = m_CellPhases[i];
      index = i * 3;
      m_CellMisorientationColors.setValue(index, 0);
      m_CellMisorientationColors.setValue(index + 1, 0);
      m_CellMisorientationColors.setValue(index + 2, 0);
      dEuler[0] = m_CellEulerAngles.getValue(index);
      dEuler[1] = m_CellEulerAngles.getValue(index + 1);
      dEuler[2] = m_CellEulerAngles.getValue(index + 2);

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
        argb = ops[m_CrystalStructures[phase]]->generateMisorientationColor(dEuler.data(), refDir.data(), false);
        m_CellMisorientationColors.setValue(index, static_cast<uint8_t>(RgbColor::dRed(argb)));
        m_CellMisorientationColors.setValue(index + 1, static_cast<uint8_t>(RgbColor::dGreen(argb)));
        m_CellMisorientationColors.setValue(index + 2, static_cast<uint8_t>(RgbColor::dBlue(argb)));

        QuatF q1;
        if constexpr(UsingEulerAngles)
        {
          float32 quat0 = m_Quats[feature1 * 4];
          float32 quat1 = m_Quats[feature1 * 4 + 1];
          float32 quat2 = m_Quats[feature1 * 4 + 2];
          float32 quat3 = m_Quats[feature1 * 4 + 3];
          QuatF q1(quat0, quat1, quat2, quat3);
        }
        else
        {
          QuatF q1(quat0, quat1, quat2, quat3);
        }
        OrientationF axisAngle = m_OrientationOps[phase1]->calculateMisorientation(q1, q2);

        m_CellMisorientationColors[3 * i + 0] = axisAngle[0] * (axisAngle[3] * Constants::k_180OverPiF);
        m_CellMisorientationColors[3 * i + 1] = axisAngle[1] * (axisAngle[3] * Constants::k_180OverPiF);
        m_CellMisorientationColors[3 * i + 2] = axisAngle[2] * (axisAngle[3] * Constants::k_180OverPiF);
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
  FloatVec3Type m_ReferenceAxis;
  Float32AbstractDataStore& m_CellOrientations;
  Int32AbstractDataStore& m_CellPhases;
  UInt32AbstractDataStore& m_CrystalStructures;
  int32_t m_NumPhases = 0;
  const IDataArray* m_GoodVoxels = nullptr;
  UInt8AbstractDataStore& m_CellMisorientationColors;
};
} // namespace

// -----------------------------------------------------------------------------
GenerateMisorientationColors::GenerateMisorientationColors(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, GenerateMisorientationColorsInputValues* inputValues)
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

  Float32Array& cellOrientationArray = m_DataStructure.getDataRefAs<Float32Array>((m_InputValues->useEulers) ? m_InputValues->cellEulerAnglesArrayPath : m_InputValues->cellQuatsArrayPath);
  Int32Array& phases = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->cellPhasesArrayPath);

  UInt32Array& crystalStructures = m_DataStructure.getDataRefAs<UInt32Array>(m_InputValues->crystalStructuresArrayPath);

  UInt8Array& MisorientationColors = m_DataStructure.getDataRefAs<UInt8Array>(m_InputValues->cellMisorientationColorsArrayPath);

  m_PhaseWarningCount = 0;
  size_t totalPoints = cellOrientationArray.getNumberOfTuples();

  int32_t numPhases = static_cast<int32_t>(crystalStructures.getNumberOfTuples());

  // Make sure we are dealing with a unit 1 vector.
  // FloatVec3Type normRefDir = m_InputValues->referenceDirection; // Make a copy of the reference Direction

  // MatrixMath::Normalize3x1(normRefDir[0], normRefDir[1], normRefDir[2]);

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

  if(m_InputValues->UseEulers)
  {
  dataAlg.execute(GenerateMisorientationColorsImpl<true>(this, m_InputValues->referenceAxis, cellOrientationArray, phases, crystalStructures, numPhases, goodVoxelsArray, MisorientationColors));
  }
  if(!m_InputValues->UseEulers)
  {
  dataAlg.execute(GenerateMisorientationColorsImpl<false>(this, m_InputValues->referenceAxis, cellOrientationArray, phases, crystalStructures, numPhases, goodVoxelsArray, MisorientationColors));
  }

  if(m_PhaseWarningCount > 0)
  {
    std::string message = fmt::format("The Ensemble Phase information only references {} phase(s) but {} cell(s) had a phase value greater than {}. \
This indicates a problem with the input cell phase data. DREAM.3D will give INCORRECT RESULTS.",
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
