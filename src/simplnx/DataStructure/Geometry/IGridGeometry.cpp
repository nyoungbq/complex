#include "IGridGeometry.hpp"

#include "simplnx/Utilities/DataObjectUtilities.hpp"

namespace nx::core
{
IGridGeometry::IGridGeometry(DataStructure& dataStructure, std::string name)
: IGeometry(dataStructure, std::move(name))
{
}

IGridGeometry::IGridGeometry(DataStructure& dataStructure, std::string name, IdType importId)
: IGeometry(dataStructure, std::move(name), importId)
{
}

const std::optional<IGridGeometry::IdType>& IGridGeometry::getCellDataId() const
{
  return m_CellDataId;
}

AttributeMatrix* IGridGeometry::getCellData()
{
  return getDataStructureRef().getDataAs<AttributeMatrix>(m_CellDataId);
}

const AttributeMatrix* IGridGeometry::getCellData() const
{
  return getDataStructureRef().getDataAs<AttributeMatrix>(m_CellDataId);
}

AttributeMatrix& IGridGeometry::getCellDataRef()
{
  return getDataStructureRef().getDataRefAs<AttributeMatrix>(m_CellDataId.value());
}

const AttributeMatrix& IGridGeometry::getCellDataRef() const
{
  return getDataStructureRef().getDataRefAs<AttributeMatrix>(m_CellDataId.value());
}

DataPath IGridGeometry::getCellDataPath() const
{
  return getCellDataRef().getDataPaths().at(0);
}

void IGridGeometry::setCellData(const AttributeMatrix& attributeMatrix)
{
  m_CellDataId = attributeMatrix.getId();
}

void IGridGeometry::setCellData(OptionalId id)
{
  m_CellDataId = id;
}

void IGridGeometry::checkUpdatedIdsImpl(const std::vector<std::pair<IdType, IdType>>& updatedIds)
{
  IGeometry::checkUpdatedIdsImpl(updatedIds);

  std::vector<bool> visited(1, false);

  for(const auto& updatedId : updatedIds)
  {
    m_CellDataId = nx::core::VisitDataStructureId(m_CellDataId, updatedId, visited, 0);
    if(visited[0])
    {
      break;
    }
  }
}
} // namespace nx::core
