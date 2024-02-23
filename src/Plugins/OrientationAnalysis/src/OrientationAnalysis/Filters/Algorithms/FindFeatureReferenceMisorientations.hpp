#pragma once

#include "OrientationAnalysis/OrientationAnalysis_export.hpp"

#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/Filter/IFilter.hpp"
#include "simplnx/Parameters/ChoicesParameter.hpp"
#include "simplnx/Common/Array.hpp"

namespace nx::core
{

struct ORIENTATIONANALYSIS_EXPORT FindFeatureReferenceMisorientationsInputValues
{
  ChoicesParameter::ValueType ReferenceOrientation;
  std::vector<float32> ConstantRefOrientationVec;
  DataPath FeatureIdsArrayPath;
  DataPath CellPhasesArrayPath;
  DataPath QuatsArrayPath;
  DataPath GBEuclideanDistancesArrayPath;
  DataPath AvgQuatsArrayPath;
  DataPath CrystalStructuresArrayPath;
  DataPath FeatureReferenceMisorientationsArrayPath;
  DataPath FeatureAvgMisorientationsArrayPath;
};

/**
 * @class ConditionalSetValue
 * @brief This filter replaces values in the target array with a user specified value
 * where a bool mask array specifies.
 */

class ORIENTATIONANALYSIS_EXPORT FindFeatureReferenceMisorientations
{
public:
  FindFeatureReferenceMisorientations(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                      FindFeatureReferenceMisorientationsInputValues* inputValues);
  ~FindFeatureReferenceMisorientations() noexcept;

  FindFeatureReferenceMisorientations(const FindFeatureReferenceMisorientations&) = delete;
  FindFeatureReferenceMisorientations(FindFeatureReferenceMisorientations&&) noexcept = delete;
  FindFeatureReferenceMisorientations& operator=(const FindFeatureReferenceMisorientations&) = delete;
  FindFeatureReferenceMisorientations& operator=(FindFeatureReferenceMisorientations&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

private:
  DataStructure& m_DataStructure;
  const FindFeatureReferenceMisorientationsInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
};

} // namespace nx::core
