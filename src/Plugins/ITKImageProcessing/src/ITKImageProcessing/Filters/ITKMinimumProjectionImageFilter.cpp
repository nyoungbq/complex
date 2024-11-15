#include "ITKMinimumProjectionImageFilter.hpp"

#include "ITKImageProcessing/Common/ITKArrayHelper.hpp"
#include "ITKImageProcessing/Common/sitkCommon.hpp"

#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Parameters/GeometrySelectionParameter.hpp"
#include "simplnx/Parameters/NumberParameter.hpp"

#include <itkMinimumProjectionImageFilter.h>

using namespace nx::core;

namespace cxITKMinimumProjectionImageFilter
{
using ArrayOptionsType = ITK::ScalarPixelIdTypeList;
// VectorPixelIDTypeList;
template <class PixelT>
using FilterOutputType = double;

struct ITKMinimumProjectionImageFilterFunctor
{
  uint32 projectionDimension = 0u;

  template <class InputImageT, class OutputImageT, uint32 Dimension>
  auto createFilter() const
  {
    using FilterType = itk::MinimumProjectionImageFilter<InputImageT, OutputImageT>;
    auto filter = FilterType::New();
    filter->SetProjectionDimension(projectionDimension);
    return filter;
  }
};
} // namespace cxITKMinimumProjectionImageFilter

namespace nx::core
{
//------------------------------------------------------------------------------
std::string ITKMinimumProjectionImageFilter::name() const
{
  return FilterTraits<ITKMinimumProjectionImageFilter>::name;
}

//------------------------------------------------------------------------------
std::string ITKMinimumProjectionImageFilter::className() const
{
  return FilterTraits<ITKMinimumProjectionImageFilter>::className;
}

//------------------------------------------------------------------------------
Uuid ITKMinimumProjectionImageFilter::uuid() const
{
  return FilterTraits<ITKMinimumProjectionImageFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string ITKMinimumProjectionImageFilter::humanName() const
{
  return "ITK Mean Projection Image Filter";
}

//------------------------------------------------------------------------------
std::vector<std::string> ITKMinimumProjectionImageFilter::defaultTags() const
{
  return {className(), "ITKImageProcessing", "ITKMinimumProjectionImageFilter", "ITKImageStatistics", "ImageStatistics"};
}

//------------------------------------------------------------------------------
Parameters ITKMinimumProjectionImageFilter::parameters() const
{
  Parameters params;
  params.insertSeparator(Parameters::Separator{"Input Parameter(s)"});
  params.insert(std::make_unique<UInt32Parameter>(k_ProjectionDimension_Key, "Projection Dimension", "The dimension index to project. 0=Slowest moving dimension.", 0u));

  params.insertSeparator(Parameters::Separator{"Input Cell Data"});
  params.insert(std::make_unique<GeometrySelectionParameter>(k_InputImageGeomPath_Key, "Image Geometry", "Select the Image Geometry Group from the DataStructure.", DataPath({"Image Geometry"}),
                                                             GeometrySelectionParameter::AllowedTypes{IGeometry::Type::Image}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_InputImageDataPath_Key, "Input Cell Data", "The image data that will be processed by this filter.", DataPath{},
                                                          nx::core::ITK::GetScalarPixelAllowedTypes()));

  params.insertSeparator(Parameters::Separator{"Output Cell Data"});
  params.insert(
      std::make_unique<DataObjectNameParameter>(k_OutputImageArrayName_Key, "Output Image Data Array", "The result of the processing will be stored in this Data Array.", "Output Image Data"));

  return params;
}

//------------------------------------------------------------------------------
IFilter::VersionType ITKMinimumProjectionImageFilter::parametersVersion() const
{
  return 1;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer ITKMinimumProjectionImageFilter::clone() const
{
  return std::make_unique<ITKMinimumProjectionImageFilter>();
}

//------------------------------------------------------------------------------
IFilter::PreflightResult ITKMinimumProjectionImageFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler,
                                                                     const std::atomic_bool& shouldCancel) const
{
  auto imageGeomPath = filterArgs.value<DataPath>(k_InputImageGeomPath_Key);
  auto selectedInputArray = filterArgs.value<DataPath>(k_InputImageDataPath_Key);
  auto outputArrayName = filterArgs.value<DataObjectNameParameter::ValueType>(k_OutputImageArrayName_Key);
  auto projectionDimension = filterArgs.value<uint32>(k_ProjectionDimension_Key);
  const DataPath outputArrayPath = selectedInputArray.replaceName(outputArrayName);

  Result<OutputActions> resultOutputActions =
      ITK::DataCheck<cxITKMinimumProjectionImageFilter::ArrayOptionsType, cxITKMinimumProjectionImageFilter::FilterOutputType>(dataStructure, selectedInputArray, imageGeomPath, outputArrayPath);

  return {std::move(resultOutputActions)};
}

//------------------------------------------------------------------------------
Result<> ITKMinimumProjectionImageFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                                   const std::atomic_bool& shouldCancel) const
{
  auto imageGeomPath = filterArgs.value<DataPath>(k_InputImageGeomPath_Key);
  auto selectedInputArray = filterArgs.value<DataPath>(k_InputImageDataPath_Key);
  auto outputArrayName = filterArgs.value<DataObjectNameParameter::ValueType>(k_OutputImageArrayName_Key);
  const DataPath outputArrayPath = selectedInputArray.replaceName(outputArrayName);

  auto projectionDimension = filterArgs.value<uint32>(k_ProjectionDimension_Key);

  const cxITKMinimumProjectionImageFilter::ITKMinimumProjectionImageFilterFunctor itkFunctor = {projectionDimension};

  auto& imageGeom = dataStructure.getDataRefAs<ImageGeom>(imageGeomPath);

  return ITK::Execute<cxITKMinimumProjectionImageFilter::ArrayOptionsType, cxITKMinimumProjectionImageFilter::FilterOutputType>(dataStructure, selectedInputArray, imageGeomPath, outputArrayPath, itkFunctor,
                                                                                                                          shouldCancel);
}
} // namespace nx::core
