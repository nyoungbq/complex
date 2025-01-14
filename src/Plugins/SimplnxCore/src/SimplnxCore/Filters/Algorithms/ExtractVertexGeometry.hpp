#pragma once

#include "SimplnxCore/SimplnxCore_export.hpp"

#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/Filter/IFilter.hpp"
#include "simplnx/Parameters/ChoicesParameter.hpp"
#include "simplnx/Parameters/MultiArraySelectionParameter.hpp"

namespace nx::core
{

struct SIMPLNXCORE_EXPORT ExtractVertexGeometryInputValues
{
  ChoicesParameter::ValueType ArrayHandling;
  bool UseMask;
  DataPath MaskArrayPath;
  DataPath InputGeometryPath;
  MultiArraySelectionParameter::ValueType IncludedDataArrayPaths;
  DataPath VertexGeometryPath;
  std::string VertexAttrMatrixName;
  std::string SharedVertexListName;
};

class SIMPLNXCORE_EXPORT ExtractVertexGeometry
{
public:
  ExtractVertexGeometry(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, ExtractVertexGeometryInputValues* inputValues);
  ~ExtractVertexGeometry() noexcept;

  ExtractVertexGeometry(const ExtractVertexGeometry&) = delete;
  ExtractVertexGeometry(ExtractVertexGeometry&&) noexcept = delete;
  ExtractVertexGeometry& operator=(const ExtractVertexGeometry&) = delete;
  ExtractVertexGeometry& operator=(ExtractVertexGeometry&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

private:
  DataStructure& m_DataStructure;
  const ExtractVertexGeometryInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
};

} // namespace nx::core
