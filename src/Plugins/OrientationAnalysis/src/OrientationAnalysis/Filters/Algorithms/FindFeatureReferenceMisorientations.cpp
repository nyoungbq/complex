#include "FindFeatureReferenceMisorientations.hpp"

#include "simplnx/Common/Numbers.hpp"
#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataGroup.hpp"
#include "simplnx/Utilities/DataArrayUtilities.hpp"

#include "EbsdLib/LaueOps/LaueOps.h"
#include "EbsdLib/Core/OrientationTransformation.hpp"

using namespace nx::core;

// -----------------------------------------------------------------------------
FindFeatureReferenceMisorientations::FindFeatureReferenceMisorientations(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                                                         FindFeatureReferenceMisorientationsInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
{
}

// -----------------------------------------------------------------------------
FindFeatureReferenceMisorientations::~FindFeatureReferenceMisorientations() noexcept = default;

// -----------------------------------------------------------------------------
const std::atomic_bool& FindFeatureReferenceMisorientations::getCancel()
{
  return m_ShouldCancel;
}

// -----------------------------------------------------------------------------
Result<> FindFeatureReferenceMisorientations::operator()()
{

  // Input Arrays
  const auto& cellPhases = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->CellPhasesArrayPath);
  const auto& featureIds = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->FeatureIdsArrayPath);
  const auto& quats = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->QuatsArrayPath);

  const auto& avgQuats = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->AvgQuatsArrayPath);

  const auto& crystalStructures = m_DataStructure.getDataRefAs<UInt32Array>(m_InputValues->CrystalStructuresArrayPath);

  // Output Arrays
  auto& featureReferenceMisorientations = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->FeatureReferenceMisorientationsArrayPath);
  auto& avgReferenceMisorientation = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->FeatureAvgMisorientationsArrayPath);

  auto validateNumFeatResult = ValidateNumFeaturesInArray(m_DataStructure, m_InputValues->FeatureAvgMisorientationsArrayPath, featureIds);
  if(validateNumFeatResult.invalid())
  {
    return validateNumFeatResult;
  }

  std::vector<LaueOps::Pointer> m_OrientationOps = LaueOps::GetAllOrientationOps();

  size_t totalPoints = featureIds.getNumberOfTuples();
  size_t totalFeatures = avgQuats.getNumberOfTuples();

  std::vector<size_t> m_Centers(totalFeatures, 0);
  std::vector<float> m_CenterDists(totalFeatures, 0.0f);
  if(m_InputValues->ReferenceOrientation == 1)
  {
    const auto& m_GBEuclideanDistances = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->GBEuclideanDistancesArrayPath);
    for(size_t i = 0; i < totalPoints; i++)
    {
      int32_t gnum = featureIds[i];
      float32 dist = m_GBEuclideanDistances[i];
      if(dist >= m_CenterDists[gnum])
      {
        m_CenterDists[gnum] = dist;
        m_Centers[gnum] = i;
      }
    }
  }

  std::vector<float> avgMiso(totalFeatures * 2, 0.0F);

  QuatF q2;
  std::vector<int32> featurePhases(totalFeatures, -1);
  if(m_InputValues->ReferenceOrientation == 2)
  {
    q2 = OrientationTransformation::ax2qu<std::vector<float32>, QuatF>(m_InputValues->ConstantRefOrientationVec);
  }
  for(int64_t point = 0; point < totalPoints; point++)
  {
    if(featureIds[point] > 0 && cellPhases[point] > 0)
    {
      QuatF q1(quats[point * 4 + 0], quats[point * 4 + 1], quats[point * 4 + 2], quats[point * 4 + 3]);
      uint32 phase1 = crystalStructures[cellPhases[point]];
      if(m_InputValues->ReferenceOrientation == 2)
      {
        featurePhases[featureIds[point]] = phase1;
      }
      if(m_InputValues->ReferenceOrientation == 0)
      {
        auto gnum = static_cast<size_t>(featureIds[point]);
        q2 = QuatF(avgQuats[gnum * 4 + 0], avgQuats[gnum * 4 + 1], avgQuats[gnum * 4 + 2], avgQuats[gnum * 4 + 3]);
      }
      else if(m_InputValues->ReferenceOrientation == 1)
      {
        auto gnum = static_cast<size_t>(featureIds[point]);
        size_t centerGNum = m_Centers[gnum];
        q2 = QuatF(avgQuats[centerGNum * 4 + 0], avgQuats[centerGNum * 4 + 1], avgQuats[centerGNum * 4 + 2], avgQuats[centerGNum * 4 + 3]);
      }

      OrientationD axisAngle = m_OrientationOps[phase1]->calculateMisorientation(q1, q2);

      featureReferenceMisorientations[point] = static_cast<float>((180.0 / nx::core::numbers::pi) * axisAngle[3]); // convert to degrees
      int32_t idx = featureIds[point] * 2;
      avgMiso[idx + 0]++;
      avgMiso[idx + 1] = avgMiso[idx + 1] + featureReferenceMisorientations[point];
    }
    if(featureIds[point] == 0 || cellPhases[point] == 0)
    {
      featureReferenceMisorientations[point] = 0.0f;
    }
  }

  if(m_InputValues->ReferenceOrientation == 2)
  {
    for(size_t i = 1; i < totalFeatures; i++)
    {
      QuatF q1 = QuatF(avgQuats[i * 4 + 0], avgQuats[i * 4 + 1], avgQuats[i * 4 + 2], avgQuats[i * 4 + 3]);

      // q2 is the constant axis angle represented as a quaternion
      OrientationD axisAngle = m_OrientationOps[featurePhases[i]]->calculateMisorientation(q1, q2);

      avgReferenceMisorientation[i] = static_cast<float>((180.0 / nx::core::numbers::pi) * axisAngle[3]); // convert to degrees
    }
  }
  else
  {
    for(size_t i = 1; i < totalFeatures; i++)
    {
      auto idx = static_cast<int32>(i * 2);
      if(avgMiso[idx] == 0.0f)
      {
        avgReferenceMisorientation[i] = 0.0f;
      }
      else
      {
        avgReferenceMisorientation[i] = avgMiso[idx + 1] / avgMiso[idx];
      }
    }
  }
  return {};
}
