#include "OrientationAnalysis/Filters/ComputeFeatureFaceMisorientationFilter.hpp"
#include "OrientationAnalysis/Filters/ConvertOrientationsFilter.hpp"
#include "OrientationAnalysis/OrientationAnalysis_test_dirs.hpp"

#include "simplnx/Parameters/ArrayCreationParameter.hpp"
#include "simplnx/UnitTest/UnitTestCommon.hpp"

#include <catch2/catch.hpp>

using namespace nx::core;
using namespace nx::core::UnitTest;

namespace
{
constexpr StringLiteral k_FaceMisorientationColors("SurfaceMeshFaceMisorientationColors");
constexpr StringLiteral k_NXFaceMisorientationColors("NXFaceMisorientationColors");
constexpr StringLiteral k_AvgQuats("AvgQuats");

DataPath smallIn100Group({nx::core::Constants::k_SmallIN100});
DataPath featureDataPath = smallIn100Group.createChildPath(nx::core::Constants::k_Grain_Data);
DataPath avgEulerAnglesPath = featureDataPath.createChildPath(nx::core::Constants::k_AvgEulerAngles);
DataPath featurePhasesPath = featureDataPath.createChildPath(nx::core::Constants::k_Phases);
DataPath crystalStructurePath = smallIn100Group.createChildPath(nx::core::Constants::k_Phase_Data).createChildPath(nx::core::Constants::k_CrystalStructures);
DataPath avgQuatsPath = featureDataPath.createChildPath(k_AvgQuats);

DataPath triangleDataContainerPath({nx::core::Constants::k_TriangleDataContainerName});
DataPath faceDataGroup = triangleDataContainerPath.createChildPath(nx::core::Constants::k_FaceData);

DataPath faceLabels = faceDataGroup.createChildPath(nx::core::Constants::k_FaceLabels);
DataPath faceNormals = faceDataGroup.createChildPath(nx::core::Constants::k_FaceNormals);
DataPath faceAreas = faceDataGroup.createChildPath(nx::core::Constants::k_FaceAreas);
} // namespace

TEST_CASE("OrientationAnalysis::ComputeFeatureFaceMisorientationFilter: Valid filter execution", "[OrientationAnalysis][ComputeFeatureFaceMisorientationFilter]")
{
  Application::GetOrCreateInstance()->loadPlugins(unit_test::k_BuildDir.view(), true);

  const nx::core::UnitTest::TestFileSentinel testDataSentinel(nx::core::unit_test::k_CMakeExecutable, nx::core::unit_test::k_TestFilesDir, "6_6_Small_IN100_GBCD.tar.gz", "6_6_Small_IN100_GBCD");

  // Read the Small IN100 Data set
  auto baseDataFilePath = fs::path(fmt::format("{}/6_6_Small_IN100_GBCD/6_6_Small_IN100_GBCD.dream3d", unit_test::k_TestFilesDir));
  DataStructure dataStructure = UnitTest::LoadDataStructure(baseDataFilePath);

  // Convert the AvgEulerAngles array to AvgQuats for use in ComputeFeatureFaceMisorientationFilter input
  {
    // Instantiate the filter, and an Arguments Object
    ConvertOrientationsFilter filter;
    Arguments args;

    // Create default Parameters for the filter.
    args.insertOrAssign(ConvertOrientationsFilter::k_InputType_Key, std::make_any<uint64>(0));
    args.insertOrAssign(ConvertOrientationsFilter::k_OutputType_Key, std::make_any<uint64>(2));
    args.insertOrAssign(ConvertOrientationsFilter::k_InputOrientationArrayPath_Key, std::make_any<DataPath>(avgEulerAnglesPath));
    args.insertOrAssign(ConvertOrientationsFilter::k_OutputOrientationArrayName_Key, std::make_any<std::string>(k_AvgQuats));

    // Execute the filter and check the result
    auto executeResult = filter.execute(dataStructure, args);
    SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result);
  }

  // ComputeFeatureFaceMisorientationFilter
  {
    // Instantiate the filter, and an Arguments Object
    ComputeFeatureFaceMisorientationFilter filter;
    Arguments args;

    // Create default Parameters for the filter.
    args.insertOrAssign(ComputeFeatureFaceMisorientationFilter::k_SurfaceMeshFaceLabelsArrayPath_Key, std::make_any<DataPath>(faceLabels));
    args.insertOrAssign(ComputeFeatureFaceMisorientationFilter::k_AvgQuatsArrayPath_Key, std::make_any<DataPath>(avgQuatsPath));
    args.insertOrAssign(ComputeFeatureFaceMisorientationFilter::k_FeaturePhasesArrayPath_Key, std::make_any<DataPath>(featurePhasesPath));
    args.insertOrAssign(ComputeFeatureFaceMisorientationFilter::k_CrystalStructuresArrayPath_Key, std::make_any<DataPath>(crystalStructurePath));
    args.insertOrAssign(ComputeFeatureFaceMisorientationFilter::k_SurfaceMeshFaceMisorientationColorsArrayName_Key, std::make_any<std::string>(::k_NXFaceMisorientationColors));

    // Preflight the filter and check result
    auto preflightResult = filter.preflight(dataStructure, args);
    SIMPLNX_RESULT_REQUIRE_VALID(preflightResult.outputActions);

    // Execute the filter and check the result
    auto executeResult = filter.execute(dataStructure, args);
    SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result);
  }

  // compare the resulting face IPF Colors array
  DataPath exemplarPath = faceDataGroup.createChildPath(::k_FaceMisorientationColors);
  DataPath generatedPath = faceDataGroup.createChildPath(::k_NXFaceMisorientationColors);
  CompareArrays<float32>(dataStructure, exemplarPath, generatedPath);
}

TEST_CASE("OrientationAnalysis::ComputeFeatureFaceMisorientationFilter: Invalid filter execution", "[OrientationAnalysis][ComputeFeatureFaceMisorientationFilter]")
{
  Application::GetOrCreateInstance()->loadPlugins(unit_test::k_BuildDir.view(), true);

  const nx::core::UnitTest::TestFileSentinel testDataSentinel(nx::core::unit_test::k_CMakeExecutable, nx::core::unit_test::k_TestFilesDir, "6_6_Small_IN100_GBCD.tar.gz", "6_6_Small_IN100_GBCD");

  // Read the Small IN100 Data set
  auto baseDataFilePath = fs::path(fmt::format("{}/6_6_Small_IN100_GBCD/6_6_Small_IN100_GBCD.dream3d", unit_test::k_TestFilesDir));
  DataStructure dataStructure = UnitTest::LoadDataStructure(baseDataFilePath);

  // Instantiate the filter, a DataStructure object and an Arguments Object
  ComputeFeatureFaceMisorientationFilter filter;
  Arguments args;

  SECTION("Inconsistent cell data tuple dimensions")
  {
    // Convert the AvgEulerAngles array to AvgQuats for use in ComputeFeatureFaceMisorientationFilter input
    {
      // Instantiate the filter, and an Arguments Object
      ConvertOrientationsFilter convertOrientationsFilter;
      Arguments convertOrientationsArgs;

      // Create default Parameters for the filter.
      convertOrientationsArgs.insertOrAssign(ConvertOrientationsFilter::k_InputType_Key, std::make_any<uint64>(0));
      convertOrientationsArgs.insertOrAssign(ConvertOrientationsFilter::k_OutputType_Key, std::make_any<uint64>(2));
      convertOrientationsArgs.insertOrAssign(ConvertOrientationsFilter::k_InputOrientationArrayPath_Key, std::make_any<DataPath>(avgEulerAnglesPath));
      convertOrientationsArgs.insertOrAssign(ConvertOrientationsFilter::k_OutputOrientationArrayName_Key, std::make_any<std::string>(k_AvgQuats));

      // Execute the filter and check the result
      auto convertOrientationsResult = convertOrientationsFilter.execute(dataStructure, convertOrientationsArgs);
      SIMPLNX_RESULT_REQUIRE_VALID(convertOrientationsResult.result);
    }

    args.insertOrAssign(ComputeFeatureFaceMisorientationFilter::k_SurfaceMeshFaceLabelsArrayPath_Key, std::make_any<DataPath>(faceLabels));
    args.insertOrAssign(ComputeFeatureFaceMisorientationFilter::k_AvgQuatsArrayPath_Key, std::make_any<DataPath>(avgQuatsPath));
    args.insertOrAssign(ComputeFeatureFaceMisorientationFilter::k_FeaturePhasesArrayPath_Key, std::make_any<DataPath>(faceAreas));
    args.insertOrAssign(ComputeFeatureFaceMisorientationFilter::k_CrystalStructuresArrayPath_Key, std::make_any<DataPath>(crystalStructurePath));
    args.insertOrAssign(ComputeFeatureFaceMisorientationFilter::k_SurfaceMeshFaceMisorientationColorsArrayName_Key, std::make_any<std::string>(::k_NXFaceMisorientationColors));
  }

  SECTION("Missing input data path")
  {
    args.insertOrAssign(ComputeFeatureFaceMisorientationFilter::k_SurfaceMeshFaceLabelsArrayPath_Key, std::make_any<DataPath>(faceLabels));
    args.insertOrAssign(ComputeFeatureFaceMisorientationFilter::k_AvgQuatsArrayPath_Key, std::make_any<DataPath>(avgQuatsPath));
    args.insertOrAssign(ComputeFeatureFaceMisorientationFilter::k_FeaturePhasesArrayPath_Key, std::make_any<DataPath>(faceAreas));
    args.insertOrAssign(ComputeFeatureFaceMisorientationFilter::k_CrystalStructuresArrayPath_Key, std::make_any<DataPath>(crystalStructurePath));
    args.insertOrAssign(ComputeFeatureFaceMisorientationFilter::k_SurfaceMeshFaceMisorientationColorsArrayName_Key, std::make_any<std::string>(::k_NXFaceMisorientationColors));
  }

  // Preflight the filter and check result
  auto preflightResult = filter.preflight(dataStructure, args);
  SIMPLNX_RESULT_REQUIRE_INVALID(preflightResult.outputActions);

  // Execute the filter and check the result
  auto executeResult = filter.execute(dataStructure, args);
  SIMPLNX_RESULT_REQUIRE_INVALID(executeResult.result);
}
