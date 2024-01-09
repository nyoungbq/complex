#include "AlignSectionsListFilter.hpp"

#include "SimplnxCore/Filters/Algorithms/AlignSectionsList.hpp"

#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/Geometry/ImageGeom.hpp"
#include "simplnx/Filter/Actions/EmptyAction.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/FileSystemPathParameter.hpp"
#include "simplnx/Parameters/GeometrySelectionParameter.hpp"
#include "simplnx/Utilities/FilterUtilities.hpp"

#include "simplnx/Utilities/SIMPLConversion.hpp"

#include <filesystem>

namespace fs = std::filesystem;
using namespace nx::core;

namespace nx::core
{
//------------------------------------------------------------------------------
std::string AlignSectionsListFilter::name() const
{
  return FilterTraits<AlignSectionsListFilter>::name.str();
}

//------------------------------------------------------------------------------
std::string AlignSectionsListFilter::className() const
{
  return FilterTraits<AlignSectionsListFilter>::className;
}

//------------------------------------------------------------------------------
Uuid AlignSectionsListFilter::uuid() const
{
  return FilterTraits<AlignSectionsListFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string AlignSectionsListFilter::humanName() const
{
  return "Align Sections (List)";
}

//------------------------------------------------------------------------------
std::vector<std::string> AlignSectionsListFilter::defaultTags() const
{
  return {className(), "Reconstruction", "Alignment"};
}

//------------------------------------------------------------------------------
Parameters AlignSectionsListFilter::parameters() const
{
  Parameters params;
  // Create the parameter descriptors that are needed for this filter
  params.insertSeparator(Parameters::Separator{"Input Parameters"});
  params.insert(std::make_unique<FileSystemPathParameter>(k_InputFile_Key, "Input File", "The input .txt file path containing the shifts to apply to the sections", fs::path("DefaultInputFileName"),
                                                          FileSystemPathParameter::ExtensionsType{"txt"}, FileSystemPathParameter::PathType::InputFile));
  params.insert(
      std::make_unique<BoolParameter>(k_DREAM3DAlignmentFile_Key, "DREAM3D Alignment File Format", "Turn this ON if the alignment file was generated by another DREAM.3D Alignment filter", false));
  params.insertSeparator(Parameters::Separator{"Required Geometry"});
  params.insert(std::make_unique<GeometrySelectionParameter>(k_SelectedImageGeometry_Key, "Selected Image Geometry", "The target geometry on which to perform the alignment",
                                                             DataPath({"Data Container"}), GeometrySelectionParameter::AllowedTypes{IGeometry::Type::Image}));
  return params;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer AlignSectionsListFilter::clone() const
{
  return std::make_unique<AlignSectionsListFilter>();
}

//------------------------------------------------------------------------------
IFilter::PreflightResult AlignSectionsListFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler,
                                                                const std::atomic_bool& shouldCancel) const
{
  auto pInputFileValue = filterArgs.value<FileSystemPathParameter::ValueType>(k_InputFile_Key);
  auto pDREAM3DAlignmentFileValue = filterArgs.value<bool>(k_DREAM3DAlignmentFile_Key);
  auto pSelectedImageGeometryPathValue = filterArgs.value<DataPath>(k_SelectedImageGeometry_Key);

  PreflightResult preflightResult;
  nx::core::Result<OutputActions> resultOutputActions;
  std::vector<PreflightValue> preflightUpdatedValues;

  const ImageGeom& imageGeom = dataStructure.getDataRefAs<ImageGeom>(pSelectedImageGeometryPathValue);
  if(imageGeom.getCellData() == nullptr)
  {
    return {MakeErrorResult<OutputActions>(-8940, fmt::format("Cannot find cell data Attribute Matrix in the selected Image geometry '{}'", pSelectedImageGeometryPathValue.toString()))};
  }

  if(imageGeom.getNumXCells() <= 1 || imageGeom.getNumYCells() <= 1 || imageGeom.getNumZCells() <= 1)
  {
    return {MakeErrorResult<OutputActions>(-8941, fmt::format("The Image Geometry is not 3D and cannot be run through this filter. The dimensions are ({},{},{})", imageGeom.getNumXCells(),
                                                              imageGeom.getNumYCells(), imageGeom.getNumZCells()))};
  }

  // Inform users that the following arrays are going to be modified in place
  // Cell Data is going to be modified
  nx::core::AppendDataObjectModifications(dataStructure, resultOutputActions.value().modifiedActions, imageGeom.getCellDataPath(), {});

  return {std::move(resultOutputActions), std::move(preflightUpdatedValues)};
}

//------------------------------------------------------------------------------
Result<> AlignSectionsListFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                              const std::atomic_bool& shouldCancel) const
{
  AlignSectionsListInputValues inputValues;

  inputValues.InputFile = filterArgs.value<FileSystemPathParameter::ValueType>(k_InputFile_Key);
  inputValues.DREAM3DAlignmentFile = filterArgs.value<bool>(k_DREAM3DAlignmentFile_Key);
  inputValues.ImageGeometryPath = filterArgs.value<DataPath>(k_SelectedImageGeometry_Key);

  return AlignSectionsList(dataStructure, messageHandler, shouldCancel, &inputValues)();
}
} // namespace nx::core

namespace
{
namespace SIMPL
{
constexpr StringLiteral k_InputFileKey = "InputFile";
constexpr StringLiteral k_DREAM3DAlignmentFileKey = "DREAM3DAlignmentFile";
constexpr StringLiteral k_CellAttributeMatrixPathKey = "CellAttributeMatrixPath";
} // namespace SIMPL

namespace SIMPLConversionCustom
{
struct AttributeMatrixSelectionFilterParameterConverter
{
  using ParameterType = AttributeMatrixSelectionParameter;
  using ValueType = ParameterType::ValueType;

  static Result<ValueType> convert(const nlohmann::json& json)
  {
    auto dataContainerNameResult = SIMPLConversion::ReadDataContainerName(json, "AttributeMatrixSelectionFilterParameter");
    if(dataContainerNameResult.invalid())
    {
      return ConvertInvalidResult<ValueType>(std::move(dataContainerNameResult));
    }

    DataPath dataPath({std::move(dataContainerNameResult.value())});

    return {std::move(dataPath)};
  }
};
} // namespace SIMPLConversionCustom
} // namespace

Result<Arguments> AlignSectionsListFilter::FromSIMPLJson(const nlohmann::json& json)
{
  Arguments args = AlignSectionsListFilter().getDefaultArguments();

  std::vector<Result<>> results;

  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::InputFileFilterParameterConverter>(args, json, SIMPL::k_InputFileKey, k_InputFile_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::BooleanFilterParameterConverter>(args, json, SIMPL::k_DREAM3DAlignmentFileKey, k_DREAM3DAlignmentFile_Key));
  results.push_back(
      SIMPLConversion::ConvertParameter<SIMPLConversionCustom::AttributeMatrixSelectionFilterParameterConverter>(args, json, SIMPL::k_CellAttributeMatrixPathKey, k_SelectedImageGeometry_Key));

  Result<> conversionResult = MergeResults(std::move(results));

  return ConvertResultTo<Arguments>(std::move(conversionResult), std::move(args));
}