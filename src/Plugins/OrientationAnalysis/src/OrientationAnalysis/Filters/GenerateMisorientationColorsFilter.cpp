#include "GenerateMisorientationColorsFilter.hpp"

#include "OrientationAnalysis/Filters/Algorithms/GenerateMisorientationColors.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/Filter/Actions/CreateArrayAction.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"

#include "simplnx/Utilities/SIMPLConversion.hpp"

#include "simplnx/Parameters/VectorParameter.hpp"

using namespace nx::core;
namespace
{

using FeatureIdsArrayType = Int32Array;
using GoodVoxelsArrayType = BoolArray;

inline constexpr int32 k_MissingGeomError = -71440;
inline constexpr int32 k_IncorrectInputArray = -71441;
inline constexpr int32 k_MissingInputArray = -71442;
inline constexpr int32 k_MissingOrIncorrectGoodVoxelsArray = -71443;
} // namespace

namespace nx::core
{
//------------------------------------------------------------------------------
std::string GenerateMisorientationColorsFilter::name() const
{
  return FilterTraits<GenerateMisorientationColorsFilter>::name.str();
}

//------------------------------------------------------------------------------
std::string GenerateMisorientationColorsFilter::className() const
{
  return FilterTraits<GenerateMisorientationColorsFilter>::className;
}

//------------------------------------------------------------------------------
Uuid GenerateMisorientationColorsFilter::uuid() const
{
  return FilterTraits<GenerateMisorientationColorsFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string GenerateMisorientationColorsFilter::humanName() const
{
  return "Generate Misorientation Colors";
}

//------------------------------------------------------------------------------
std::vector<std::string> GenerateMisorientationColorsFilter::defaultTags() const
{
  return {className(), "Processing", "Crystallography", "Inverse Pole Figure", "Colors"};
}

//------------------------------------------------------------------------------
Parameters GenerateMisorientationColorsFilter::parameters() const
{
  Parameters params;

  params.insertSeparator(Parameters::Separator{"Input Parameters"});
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_UseEulers_Key, "Use Euler Angles", "If true the algorithm will take euler angles and do additional conversions, else the algorithm will require quats array", true));
  params.insert(std::make_unique<VectorFloat32Parameter>(k_ReferenceAxis_Key, "Reference Axis", "The reference axis with respect to compute the Misorientation colors in Axis-Angle representation",
                                                         std::vector<float32>{0.0f, 0.0f, 0.0f, 0.0f}, std::vector<std::string>({"x", "y", "z", "theta"})));

  params.insertSeparator(Parameters::Separator{"Optional Data Mask"});
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_UseMask_Key, "Use Mask Array", "Whether to assign a black color to 'bad' Elements", false));
  params.insert(std::make_unique<ArraySelectionParameter>(k_MaskArrayPath_Key, "Mask Array", "Path to the data array used to define Elements as good or bad.", DataPath(),
                                                          ArraySelectionParameter::AllowedTypes{DataType::boolean, DataType::uint8}, ArraySelectionParameter::AllowedComponentShapes{{1}}));
  // Associate the Linkable Parameter(s) to the children parameters that they control
  params.linkParameters(k_UseMask_Key, k_MaskArrayPath_Key, true);

  params.insertSeparator(Parameters::Separator{"Required Input Cell Data"});
  params.insert(std::make_unique<ArraySelectionParameter>(k_CellEulerAnglesArrayPath_Key, "Euler Angles", "Three angles defining the orientation of the Element in Bunge convention (Z-X-Z)",
                                                          DataPath{}, ArraySelectionParameter::AllowedTypes{DataType::float32}, ArraySelectionParameter::AllowedComponentShapes{{3}}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_CellQuatsArrayPath_Key, "Quaternions", "Specifies the orientation of the Cell in quaternion representation", DataPath({"CellData", "Quats"}),
                                                          ArraySelectionParameter::AllowedTypes{DataType::float32}, ArraySelectionParameter::AllowedComponentShapes{{4}}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_CellPhasesArrayPath_Key, "Phases", "Specifies to which Ensemble each cell belongs", DataPath{},
                                                          ArraySelectionParameter::AllowedTypes{DataType::int32}, ArraySelectionParameter::AllowedComponentShapes{{1}}));
  params.insertSeparator(Parameters::Separator{"Required Input Cell Ensemble Data"});
  params.insert(std::make_unique<ArraySelectionParameter>(k_CrystalStructuresArrayPath_Key, "Crystal Structures", "Enumeration representing the crystal structure for each Ensemble",
                                                          DataPath({"Ensemble Data", "CrystalStructures"}), ArraySelectionParameter::AllowedTypes{DataType::uint32},
                                                          ArraySelectionParameter::AllowedComponentShapes{{1}}));

  params.insertSeparator(Parameters::Separator{"Created Cell Data"});
  params.insert(
      std::make_unique<DataObjectNameParameter>(k_CellMisorientationColorsArrayName_Key, "Misorientation Colors", "The name of the array containing the RGB colors encoded as unsigned chars for each Element", "MisorientationColors"));

  // associate parameters to their dynamic logic
  params.linkParameters(k_UseEulers_Key, k_CellEulerAnglesArrayPath_Key, true);
  params.linkParameters(k_UseEulers_Key, k_CellQuatsArrayPath_Key, false);

  return params;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer GenerateMisorientationColorsFilter::clone() const
{
  return std::make_unique<GenerateMisorientationColorsFilter>();
}

//------------------------------------------------------------------------------
IFilter::PreflightResult GenerateMisorientationColorsFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler,
                                                                const std::atomic_bool& shouldCancel) const
{

  auto pReferenceAxisValue = filterArgs.value<VectorFloat32Parameter::ValueType>(k_ReferenceAxis_Key);
  auto pUseGoodVoxelsValue = filterArgs.value<bool>(k_UseMask_Key);
  auto pCellEulerAnglesArrayPathValue = filterArgs.value<DataPath>(k_CellEulerAnglesArrayPath_Key);
  auto pCellPhasesArrayPathValue = filterArgs.value<DataPath>(k_CellPhasesArrayPath_Key);
  auto pGoodVoxelsArrayPathValue = filterArgs.value<DataPath>(k_MaskArrayPath_Key);
  auto pCrystalStructuresArrayPathValue = filterArgs.value<DataPath>(k_CrystalStructuresArrayPath_Key);
  auto pCellMisorientationColorsArrayNameValue = pCellEulerAnglesArrayPathValue.getParent().createChildPath(filterArgs.value<std::string>(k_CellMisorientationColorsArrayName_Key));

  // Validate the Crystal Structures array
  const UInt32Array& crystalStructures = dataStructure.getDataRefAs<UInt32Array>(pCrystalStructuresArrayPathValue);
  if(crystalStructures.getNumberOfComponents() != 1)
  {
    return {nonstd::make_unexpected(std::vector<Error>{Error{k_IncorrectInputArray, "Crystal Structures Input Array must be a 1 component Int32 array"}})};
  }

  std::vector<DataPath> dataPaths;

  // Validate the Eulers array
  const Float32Array& quats = dataStructure.getDataRefAs<Float32Array>(pCellEulerAnglesArrayPathValue);
  if(quats.getNumberOfComponents() != 3)
  {
    return {nonstd::make_unexpected(std::vector<Error>{Error{k_IncorrectInputArray, "Euler Angles Input Array must be a 3 component Float32 array"}})};
  }
  dataPaths.push_back(pCellEulerAnglesArrayPathValue);

  // Validate the Phases array
  const Int32Array& phases = dataStructure.getDataRefAs<Int32Array>(pCellPhasesArrayPathValue);
  if(phases.getNumberOfComponents() != 1)
  {
    return {nonstd::make_unexpected(std::vector<Error>{Error{k_IncorrectInputArray, "Phases Input Array must be a 1 component Int32 array"}})};
  }
  dataPaths.push_back(pCellPhasesArrayPathValue);

  // Validate the GoodVoxels/Mask Array combination
  DataPath goodVoxelsPath;
  if(pUseGoodVoxelsValue)
  {
    goodVoxelsPath = filterArgs.value<DataPath>(k_MaskArrayPath_Key);

    const nx::core::IDataArray* goodVoxelsArray = dataStructure.getDataAs<IDataArray>(goodVoxelsPath);
    if(nullptr == goodVoxelsArray)
    {
      return {nonstd::make_unexpected(std::vector<Error>{Error{k_MissingOrIncorrectGoodVoxelsArray, fmt::format("Mask array is not located at path: '{}'", goodVoxelsPath.toString())}})};
    }

    if(goodVoxelsArray->getDataType() != DataType::boolean && goodVoxelsArray->getDataType() != DataType::uint8)
    {
      return {nonstd::make_unexpected(
          std::vector<Error>{Error{k_MissingOrIncorrectGoodVoxelsArray, fmt::format("Mask array at path '{}' is not of the correct type. It must be Bool or UInt8", goodVoxelsPath.toString())}})};
    }
    dataPaths.push_back(goodVoxelsPath);
  }

  auto tupleValidityCheck = dataStructure.validateNumberOfTuples(dataPaths);
  if(!tupleValidityCheck)
  {
    return {MakeErrorResult<OutputActions>(-651, fmt::format("The following DataArrays all must have equal number of tuples but this was not satisfied.\n{}", tupleValidityCheck.error()))};
  }

  // Get the number of tuples
  auto* eulersArray = dataStructure.getDataAs<Float32Array>(pCellEulerAnglesArrayPathValue);

  // Create output DataStructure Items
  auto createMisorientationColorsAction = std::make_unique<CreateArrayAction>(DataType::uint8, eulersArray->getIDataStore()->getTupleShape(), std::vector<usize>{3}, pCellMisorientationColorsArrayNameValue);

  OutputActions actions;
  actions.appendAction(std::move(createMisorientationColorsAction));

  return {std::move(actions)};
}

//------------------------------------------------------------------------------
Result<> GenerateMisorientationColorsFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                              const std::atomic_bool& shouldCancel) const
{
  GenerateMisorientationColorsInputValues inputValues;

  inputValues.useEulers = filterArgs.value<bool>(k_UseEulers_Key);
  inputValues.referenceAxis = filterArgs.value<VectorFloat32Parameter::ValueType>(k_ReferenceAxis_Key);
  inputValues.useGoodVoxels = filterArgs.value<bool>(k_UseMask_Key);
  inputValues.cellEulerAnglesArrayPath = filterArgs.value<DataPath>(k_CellEulerAnglesArrayPath_Key);
  inputValues.cellQuatsArrayPath = filterArgs.value<DataPath>(k_CellQuatsArrayPath_Key);
  inputValues.cellPhasesArrayPath = filterArgs.value<DataPath>(k_CellPhasesArrayPath_Key);
  inputValues.goodVoxelsArrayPath = filterArgs.value<DataPath>(k_MaskArrayPath_Key);
  inputValues.crystalStructuresArrayPath = filterArgs.value<DataPath>(k_CrystalStructuresArrayPath_Key);
  inputValues.cellMisorientationColorsArrayPath = inputValues.cellEulerAnglesArrayPath.getParent().createChildPath(filterArgs.value<std::string>(k_CellMisorientationColorsArrayName_Key));

  // Let the Algorithm instance do the work
  return GenerateMisorientationColors(dataStructure, messageHandler, shouldCancel, &inputValues)();
}

namespace
{
namespace SIMPL
{
constexpr StringLiteral k_ReferenceDirKey = "ReferenceDir";
constexpr StringLiteral k_UseGoodVoxelsKey = "UseGoodVoxels";
constexpr StringLiteral k_CellEulerAnglesArrayPathKey = "CellEulerAnglesArrayPath";
constexpr StringLiteral k_CellPhasesArrayPathKey = "CellPhasesArrayPath";
constexpr StringLiteral k_GoodVoxelsArrayPathKey = "GoodVoxelsArrayPath";
constexpr StringLiteral k_CrystalStructuresArrayPathKey = "CrystalStructuresArrayPath";
constexpr StringLiteral k_CellMisorientationColorsArrayNameKey = "CellMisorientationColorsArrayName";
} // namespace SIMPL
} // namespace

Result<Arguments> GenerateMisorientationColorsFilter::FromSIMPLJson(const nlohmann::json& json)
{
  Arguments args = GenerateMisorientationColorsFilter().getDefaultArguments();

  std::vector<Result<>> results;

  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::LinkedBooleanFilterParameterConverter>(args, json, SIMPL::k_UseGoodVoxelsKey, k_UseMask_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_CellEulerAnglesArrayPathKey, k_CellEulerAnglesArrayPath_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_CellPhasesArrayPathKey, k_CellPhasesArrayPath_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_GoodVoxelsArrayPathKey, k_MaskArrayPath_Key));
  results.push_back(
      SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_CrystalStructuresArrayPathKey, k_CrystalStructuresArrayPath_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::LinkedPathCreationFilterParameterConverter>(args, json, SIMPL::k_CellMisorientationColorsArrayNameKey, k_CellMisorientationColorsArrayName_Key));

  Result<> conversionResult = MergeResults(std::move(results));

  return ConvertResultTo<Arguments>(std::move(conversionResult), std::move(args));
}
} // namespace nx::core
