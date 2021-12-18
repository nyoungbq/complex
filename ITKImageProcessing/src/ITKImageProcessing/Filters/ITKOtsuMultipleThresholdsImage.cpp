#include "ITKOtsuMultipleThresholdsImage.hpp"

#include "complex/DataStructure/DataPath.hpp"
#include "complex/Filter/Actions/EmptyAction.hpp"
#include "complex/Parameters/ArrayCreationParameter.hpp"
#include "complex/Parameters/ArraySelectionParameter.hpp"
#include "complex/Parameters/BoolParameter.hpp"
#include "complex/Parameters/GeometrySelectionParameter.hpp"
#include "complex/Parameters/NumberParameter.hpp"

#include "ITKImageProcessing/Common/ITKArrayHelper.hpp"

using namespace complex;

#include <itkOtsuMultipleThresholdsImageFilter.h>

namespace
{
struct ITKOtsuMultipleThresholdsImageFilterCreationFunctor
{
  int32 m_NumberOfThresholds;
  int32 m_LabelOffset;
  float64 m_NumberOfHistogramBins;
  bool m_ValleyEmphasis;
  template <typename InputImageType, typename OutputImageType, unsigned int Dimension>
  auto operator()() const
  {
    typedef itk::OtsuMultipleThresholdsImageFilter<InputImageType, OutputImageType> FilterType;
    typename FilterType::Pointer filter = FilterType::New();
    filter->SetNumberOfThresholds(static_cast<uint8_t>(m_NumberOfThresholds));
    filter->SetLabelOffset(static_cast<uint8_t>(m_LabelOffset));
    filter->SetNumberOfHistogramBins(static_cast<uint32_t>(m_NumberOfHistogramBins));
    filter->SetValleyEmphasis(static_cast<bool>(m_ValleyEmphasis));
    return filter;
  }
};
} // namespace

namespace complex
{
//------------------------------------------------------------------------------
std::string ITKOtsuMultipleThresholdsImage::name() const
{
  return FilterTraits<ITKOtsuMultipleThresholdsImage>::name.str();
}

//------------------------------------------------------------------------------
std::string ITKOtsuMultipleThresholdsImage::className() const
{
  return FilterTraits<ITKOtsuMultipleThresholdsImage>::className;
}

//------------------------------------------------------------------------------
Uuid ITKOtsuMultipleThresholdsImage::uuid() const
{
  return FilterTraits<ITKOtsuMultipleThresholdsImage>::uuid;
}

//------------------------------------------------------------------------------
std::string ITKOtsuMultipleThresholdsImage::humanName() const
{
  return "ITK::Otsu Multiple Thresholds Image Filter";
}

//------------------------------------------------------------------------------
std::vector<std::string> ITKOtsuMultipleThresholdsImage::defaultTags() const
{
  return {"#ITK Image Processing", "#ITK Thresholding"};
}

//------------------------------------------------------------------------------
Parameters ITKOtsuMultipleThresholdsImage::parameters() const
{
  Parameters params;
  // Create the parameter descriptors that are needed for this filter
  params.insert(std::make_unique<Int32Parameter>(k_NumberOfThresholds_Key, "NumberOfThresholds", "", 1234356));
  params.insert(std::make_unique<Int32Parameter>(k_LabelOffset_Key, "LabelOffset", "", 1234356));
  params.insert(std::make_unique<Float64Parameter>(k_NumberOfHistogramBins_Key, "NumberOfHistogramBins", "", 2.3456789));
  params.insert(std::make_unique<BoolParameter>(k_ValleyEmphasis_Key, "ValleyEmphasis", "", false));
  params.insert(std::make_unique<GeometrySelectionParameter>(k_SelectedImageGeomPath_Key, "Image Geometry", "", DataPath{}, GeometrySelectionParameter::AllowedTypes{DataObject::Type::ImageGeom}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_SelectedCellArrayPath_Key, "Attribute Array to filter", "", DataPath{}));
  params.insert(std::make_unique<ArrayCreationParameter>(k_NewCellArrayName_Key, "Filtered Array", "", DataPath{}));

  return params;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer ITKOtsuMultipleThresholdsImage::clone() const
{
  return std::make_unique<ITKOtsuMultipleThresholdsImage>();
}

//------------------------------------------------------------------------------
IFilter::PreflightResult ITKOtsuMultipleThresholdsImage::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler) const
{
  /****************************************************************************
   * Write any preflight sanity checking codes in this function
   ***************************************************************************/

  /**
   * These are the values that were gathered from the UI or the pipeline file or
   * otherwise passed into the filter. These are here for your convenience. If you
   * do not need some of them remove them.
   */
  auto pNumberOfThresholds = filterArgs.value<int32>(k_NumberOfThresholds_Key);
  auto pLabelOffset = filterArgs.value<int32>(k_LabelOffset_Key);
  auto pNumberOfHistogramBins = filterArgs.value<float64>(k_NumberOfHistogramBins_Key);
  auto pValleyEmphasis = filterArgs.value<bool>(k_ValleyEmphasis_Key);
  auto pImageGeomPath = filterArgs.value<DataPath>(k_SelectedImageGeomPath_Key);
  auto pSelectedCellArrayPath = filterArgs.value<DataPath>(k_SelectedCellArrayPath_Key);
  auto pOutputArrayPath = filterArgs.value<DataPath>(k_NewCellArrayName_Key);

  // Declare the preflightResult variable that will be populated with the results
  // of the preflight. The PreflightResult type contains the output Actions and
  // any preflight updated values that you want to be displayed to the user, typically
  // through a user interface (UI).
  PreflightResult preflightResult;
  // If your filter is going to pass back some `preflight updated values` then this is where you
  // would create the code to store those values in the appropriate object. Note that we
  // in line creating the pair (NOT a std::pair<>) of Key:Value that will get stored in
  // the std::vector<PreflightValue> object.
  std::vector<PreflightValue> preflightUpdatedValues;

  // If your filter is making structural changes to the DataStructure then the filter
  // is going to create OutputActions subclasses that need to be returned. This will
  // store those actions.
  complex::Result<OutputActions> resultOutputActions;

  resultOutputActions = ITK::DataCheck(dataStructure, pSelectedCellArrayPath, pImageGeomPath, pOutputArrayPath);

  // If the filter needs to pass back some updated values via a key:value string:string set of values
  // you can declare and update that string here.
  // None found in this filter based on the filter parameters

  // If this filter makes changes to the DataStructure in the form of
  // creating/deleting/moving/renaming DataGroups, Geometries, DataArrays then you
  // will need to use one of the `*Actions` classes located in complex/Filter/Actions
  // to relay that information to the preflight and execute methods. This is done by
  // creating an instance of the Action class and then storing it in the resultOutputActions variable.
  // This is done through a `push_back()` method combined with a `std::move()`. For the
  // newly initiated to `std::move` once that code is executed what was once inside the Action class
  // instance variable is *no longer there*. The memory has been moved. If you try to access that
  // variable after this line you will probably get a crash or have subtle bugs. To ensure that this
  // does not happen we suggest using braces `{}` to scope each of the action's declaration and store
  // so that the programmer is not tempted to use the action instance past where it should be used.
  // You have to create your own Actions class if there isn't something specific for your filter's needs

  // Store the preflight updated value(s) into the preflightUpdatedValues vector using
  // the appropriate methods.
  // None found based on the filter parameters

  // Return both the resultOutputActions and the preflightUpdatedValues via std::move()
  return {std::move(resultOutputActions), std::move(preflightUpdatedValues)};
}

//------------------------------------------------------------------------------
Result<> ITKOtsuMultipleThresholdsImage::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler) const
{
  /****************************************************************************
   * Extract the actual input values from the 'filterArgs' object
   ***************************************************************************/
  auto pNumberOfThresholds = filterArgs.value<int32>(k_NumberOfThresholds_Key);
  auto pLabelOffset = filterArgs.value<int32>(k_LabelOffset_Key);
  auto pNumberOfHistogramBins = filterArgs.value<float64>(k_NumberOfHistogramBins_Key);
  auto pValleyEmphasis = filterArgs.value<bool>(k_ValleyEmphasis_Key);
  auto pImageGeomPath = filterArgs.value<DataPath>(k_SelectedImageGeomPath_Key);
  auto pSelectedCellArrayPath = filterArgs.value<DataPath>(k_SelectedCellArrayPath_Key);
  auto pOutputArrayPath = filterArgs.value<DataPath>(k_NewCellArrayName_Key);

  /****************************************************************************
   * Write your algorithm implementation in this function
   ***************************************************************************/
  ::ITKOtsuMultipleThresholdsImageFilterCreationFunctor itkFunctor;
  itkFunctor.m_NumberOfThresholds = pNumberOfThresholds;
  itkFunctor.m_LabelOffset = pLabelOffset;
  itkFunctor.m_NumberOfHistogramBins = pNumberOfHistogramBins;
  itkFunctor.m_ValleyEmphasis = pValleyEmphasis;

  return ITK::Execute(dataStructure, pSelectedCellArrayPath, pImageGeomPath, pOutputArrayPath, itkFunctor);
}
} // namespace complex
