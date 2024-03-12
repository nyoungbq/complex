#pragma once

#include "OrientationAnalysis/OrientationAnalysis_export.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/DataStructure/IDataArray.hpp"
#include "simplnx/Filter/IFilter.hpp"

#include <vector>

namespace nx::core
{

/**
 * @brief The GenerateMisorientationColorsInputValues struct
 */
struct ORIENTATIONANALYSIS_EXPORT GenerateMisorientationColorsInputValues
{
  bool useEulers;
  std::vector<float> referenceAxis;
  bool useGoodVoxels;
  DataPath goodVoxelsArrayPath;
  DataPath cellPhasesArrayPath;
  DataPath cellEulerAnglesArrayPath;
  DataPath cellQuatsArrayPath;
  DataPath crystalStructuresArrayPath;
  DataPath cellMisorientationColorsArrayPath;
};

/**
 * @brief
 */
class ORIENTATIONANALYSIS_EXPORT GenerateMisorientationColors
{
public:
  GenerateMisorientationColors(DataStructure& dataStructure, const IFilter::MessageHandler& msgHandler, const std::atomic_bool& shouldCancel, GenerateMisorientationColorsInputValues* inputValues);
  ~GenerateMisorientationColors() noexcept;

  GenerateMisorientationColors(const GenerateMisorientationColors&) = delete;            // Copy Constructor Not Implemented
  GenerateMisorientationColors(GenerateMisorientationColors&&) = delete;                 // Move Constructor Not Implemented
  GenerateMisorientationColors& operator=(const GenerateMisorientationColors&) = delete; // Copy Assignment Not Implemented
  GenerateMisorientationColors& operator=(GenerateMisorientationColors&&) = delete;      // Move Assignment Not Implemented

  Result<> operator()();

  /**
   * @brief incrementPhaseWarningCount
   */
  void incrementPhaseWarningCount();

protected:
private:
  DataStructure& m_DataStructure;
  const IFilter::MessageHandler& m_MessageHandler;
  const std::atomic_bool& m_ShouldCancel;
  const GenerateMisorientationColorsInputValues* m_InputValues = nullptr;

  int32_t m_PhaseWarningCount = 0;
};

} // namespace nx::core
