#pragma once

#include "OrientationAnalysis/OrientationAnalysis_export.hpp"

#include "simplnx/Filter/FilterTraits.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{
/**
 * @class ReadH5OinaDataFilter
 * @brief This filter will read a single .h5oina file into a new Image Geometry, allowing the immediate use of Filters on the data.
 */
class ORIENTATIONANALYSIS_EXPORT ReadH5OinaDataFilter : public IFilter
{
public:
  ReadH5OinaDataFilter() = default;
  ~ReadH5OinaDataFilter() noexcept override = default;

  ReadH5OinaDataFilter(const ReadH5OinaDataFilter&) = delete;
  ReadH5OinaDataFilter(ReadH5OinaDataFilter&&) noexcept = delete;

  ReadH5OinaDataFilter& operator=(const ReadH5OinaDataFilter&) = delete;
  ReadH5OinaDataFilter& operator=(ReadH5OinaDataFilter&&) noexcept = delete;

  // Parameter Keys
  static inline constexpr StringLiteral k_SelectedScanNames_Key = "selected_scan_names";
  static inline constexpr StringLiteral k_ZSpacing_Key = "z_spacing";
  static inline constexpr StringLiteral k_Origin_Key = "origin";
  static inline constexpr StringLiteral k_ReadPatternData_Key = "read_pattern_data";
  static inline constexpr StringLiteral k_CreatedImageGeometryPath_Key = "output_image_geometry_path";
  static inline constexpr StringLiteral k_CellAttributeMatrixName_Key = "cell_attribute_matrix_name";
  static inline constexpr StringLiteral k_CellEnsembleAttributeMatrixName_Key = "cell_ensemble_attribute_matrix_name";
  static inline constexpr StringLiteral k_EdaxHexagonalAlignment_Key = "edax_hexagonal_alignment";
  static inline constexpr StringLiteral k_ConvertPhaseToInt32_Key = "convert_phase_to_int32";

  /**
   * @brief Returns the name of the filter.
   * @return
   */
  std::string name() const override;

  /**
   * @brief Returns the C++ classname of this filter.
   * @return
   */
  std::string className() const override;

  /**
   * @brief Returns the uuid of the filter.
   * @return
   */
  Uuid uuid() const override;

  /**
   * @brief Returns the human readable name of the filter.
   * @return
   */
  std::string humanName() const override;

  /**
   * @brief Returns the default tags for this filter.
   * @return
   */
  std::vector<std::string> defaultTags() const override;

  /**
   * @brief Returns the parameters of the filter (i.e. its inputs)
   * @return
   */
  Parameters parameters() const override;

  /**
   * @brief Returns a copy of the filter.
   * @return
   */
  UniquePointer clone() const override;

protected:
  /**
   * @brief Takes in a DataStructure and checks that the filter can be run on it with the given arguments.
   * Returns any warnings/errors. Also returns the changes that would be applied to the DataStructure.
   * Some parts of the actions may not be completely filled out if all the required information is not available at preflight time.
   * @param dataStructure The input DataStructure instance
   * @param filterArgs These are the input values for each parameter that is required for the filter
   * @param messageHandler The MessageHandler object
   * @return Returns a Result object with error or warning values if any of those occurred during execution of this function
   */
  PreflightResult preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel) const override;

  /**
   * @brief Applies the filter's algorithm to the DataStructure with the given arguments. Returns any warnings/errors.
   * On failure, there is no guarantee that the DataStructure is in a correct state.
   * @param ds The input DataStructure instance
   * @param filterArgs These are the input values for each parameter that is required for the filter
   * @param messageHandler The MessageHandler object
   * @return Returns a Result object with error or warning values if any of those occurred during execution of this function
   */
  Result<> executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                       const std::atomic_bool& shouldCancel) const override;
};
} // namespace nx::core

SIMPLNX_DEF_FILTER_TRAITS(nx::core, ReadH5OinaDataFilter, "fad3d47f-f1e1-4429-bc65-5e021be62ba0");
/* LEGACY UUID FOR THIS FILTER 3ff4701b-3a0c-52e3-910a-fa927aa6584c */