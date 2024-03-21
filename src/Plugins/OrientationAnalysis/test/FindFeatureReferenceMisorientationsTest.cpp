#include "OrientationAnalysis/Filters/FindFeatureReferenceMisorientationsFilter.hpp"
#include "OrientationAnalysis/OrientationAnalysis_test_dirs.hpp"

#include "simplnx/Parameters/ArrayCreationParameter.hpp"
#include "simplnx/Parameters/ChoicesParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/UnitTest/UnitTestCommon.hpp"

#include <catch2/catch.hpp>

#include <filesystem>
namespace fs = std::filesystem;

using namespace nx::core;

namespace
{
const std::string k_FeatureReferenceMisorientationsArrayName("FeatureReferenceMisorientations");
const std::string k_GBEuclideanDistancesArrayName("FeatureAvgMisorientations");

const std::string k_FeatureReferenceMisorientationsArrayName2("FeatureReferenceMisorientations2");
const std::string k_GBEuclideanDistancesArrayName2("FeatureAvgMisorientations2");

namespace ConstantTest
{
const std::string k_TestingDirName("find_feature_reference_misorientations_test");

const DataPath k_ImageGeomPath({"Misorientation Test"});

// Ensemble Attribute Matrix
const DataPath k_CrystalStructuresPath = k_ImageGeomPath.createChildPath(Constants::k_EnsembleAttributeMatrix).createChildPath(Constants::k_CrystalStructures);

// Cell Data
const DataPath k_CellData = k_ImageGeomPath.createChildPath("Cell Data");

const DataPath k_PhasesPath = k_CellData.createChildPath("phases");
const DataPath k_QuatsPath = k_CellData.createChildPath("quats");
const DataPath k_FeatureIdsPath = k_CellData.createChildPath("featureIds");

const std::string k_GeneratedFeatureMisorientationsName("GeneratedFeatureReferenceMisorientations");
const DataPath k_GeneratedCellArrayPath = k_CellData.createChildPath(k_GeneratedFeatureMisorientationsName);
const DataPath k_ExemplarCellArrayPath = k_CellData.createChildPath("ExemplarFeatureReferenceMisorientations");

// Cell Feature Data
const DataPath k_CellFeatureData = k_ImageGeomPath.createChildPath("Cell Feature Data");

const DataPath k_AvgQuatsPath = k_CellFeatureData.createChildPath(Constants::k_AvgQuats);

const std::string k_GeneratedAvgMisorientationsName("GeneratedFeatureAvgMisorientations");
const DataPath k_GeneratedCellFeatureArrayPath = k_CellFeatureData.createChildPath(k_GeneratedAvgMisorientationsName);
const DataPath k_ExemplarCellFeatureArrayPath = k_CellFeatureData.createChildPath("ExemplarFeatureAvgMisorientations");
} // namespace ConstantTest
} // namespace

TEST_CASE("OrientationAnalysis::FindFeatureReferenceMisorientationsFilter", "[OrientationAnalysis][FindFeatureReferenceMisorientationsFilter]")
{
  Application::GetOrCreateInstance()->loadPlugins(unit_test::k_BuildDir.view(), true);

  const nx::core::UnitTest::TestFileSentinel testDataSentinel(nx::core::unit_test::k_CMakeExecutable, nx::core::unit_test::k_TestFilesDir, "6_6_stats_test.tar.gz", "6_6_stats_test.dream3d");

  // Read the Small IN100 Data set
  auto baseDataFilePath = fs::path(fmt::format("{}/6_6_stats_test.dream3d", unit_test::k_TestFilesDir));
  DataStructure dataStructure = UnitTest::LoadDataStructure(baseDataFilePath);
  DataPath smallIn100Group({Constants::k_DataContainer});
  DataPath cellDataPath = smallIn100Group.createChildPath(Constants::k_CellData);

  DataPath cellFeatureDataPath({Constants::k_DataContainer, Constants::k_CellFeatureData});
  DataPath avgQuatsDataPath = cellFeatureDataPath.createChildPath(Constants::k_AvgQuats);
  DataPath featurePhasesDataPath = cellFeatureDataPath.createChildPath(Constants::k_Phases);

  // Instantiate the filter and an Arguments Object
  {
    FindFeatureReferenceMisorientationsFilter filter;
    Arguments args;

    // Create default Parameters for the filter.
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_ReferenceOrientation_Key, std::make_any<ChoicesParameter::ValueType>(0));
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_GBEuclideanDistancesArrayPath_Key, std::make_any<DataPath>(DataPath{}));

    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_CellFeatureIdsArrayPath_Key, std::make_any<DataPath>(Constants::k_FeatureIdsArrayPath));
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_CellPhasesArrayPath_Key, std::make_any<DataPath>(Constants::k_PhasesArrayPath));
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_QuatsArrayPath_Key, std::make_any<DataPath>(Constants::k_QuatsArrayPath));

    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_AvgQuatsArrayPath_Key, std::make_any<DataPath>(avgQuatsDataPath));
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_CrystalStructuresArrayPath_Key, std::make_any<DataPath>(Constants::k_CrystalStructuresArrayPath));

    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_FeatureReferenceMisorientationsArrayName_Key,
                        std::make_any<DataObjectNameParameter::ValueType>(k_FeatureReferenceMisorientationsArrayName2));
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_FeatureAvgMisorientationsArrayName_Key, std::make_any<DataObjectNameParameter::ValueType>(k_GBEuclideanDistancesArrayName2));

    // Preflight the filter and check result
    auto preflightResult = filter.preflight(dataStructure, args);
    SIMPLNX_RESULT_REQUIRE_VALID(preflightResult.outputActions);

    // Execute the filter and check the result
    auto executeResult = filter.execute(dataStructure, args);
    SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result);
  }

  // Compare the Output Cell Data
  {
    const DataPath k_GeneratedDataPath({Constants::k_DataContainer, Constants::k_CellData, k_FeatureReferenceMisorientationsArrayName2});
    const DataPath k_ExemplarArrayPath({Constants::k_DataContainer, Constants::k_CellData, k_FeatureReferenceMisorientationsArrayName});

    UnitTest::CompareArrays<float>(dataStructure, k_ExemplarArrayPath, k_GeneratedDataPath);
  }

  // Compare the Output Feature Data
  {
    const DataPath k_GeneratedDataPath({Constants::k_DataContainer, Constants::k_CellFeatureData, k_GBEuclideanDistancesArrayName2});
    const DataPath k_ExemplarArrayPath({Constants::k_DataContainer, Constants::k_CellFeatureData, k_GBEuclideanDistancesArrayName});

    UnitTest::CompareArrays<float>(dataStructure, k_ExemplarArrayPath, k_GeneratedDataPath);
  }

#ifdef SIMPLNX_WRITE_TEST_OUTPUT
  WriteTestDataStructure(dataStructure, fs::path(fmt::format("{}/find_feature_reference_misorientations.dream3d", unit_test::k_BinaryTestOutputDir)));
#endif
}

TEST_CASE("OrientationAnalysis::FindFeatureReferenceMisorientationsFilter: Constant Misorientation Z-Aligned Test", "[OrientationAnalysis][FindFeatureReferenceMisorientationsFilter]")
{
  const nx::core::UnitTest::TestFileSentinel testDataSentinel(nx::core::unit_test::k_CMakeExecutable, nx::core::unit_test::k_TestFilesDir, fmt::format("{}.tar.gz", ::ConstantTest::k_TestingDirName),
                                                              ::ConstantTest::k_TestingDirName);

  // Load DataStructure From Input File
  auto baseDataFilePath = fs::path(fmt::format("{}/{}/misorientation_z_aligned.dream3d", unit_test::k_TestFilesDir, ::ConstantTest::k_TestingDirName));
  DataStructure dataStructure = UnitTest::LoadDataStructure(baseDataFilePath);

  // Instantiate the filter and an Arguments Object
  {
    FindFeatureReferenceMisorientationsFilter filter;
    Arguments args;

    // Create default Parameters for the filter.
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_ReferenceOrientation_Key, std::make_any<ChoicesParameter::ValueType>(2));
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_ConstantRefOrientationVec_Key, std::make_any<std::vector<float32>>(std::vector<float32>{0.0f, 0.0f, 1.0f, 0.0f}));

    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_CellFeatureIdsArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_FeatureIdsPath));
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_CellPhasesArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_PhasesPath));
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_QuatsArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_QuatsPath));

    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_AvgQuatsArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_AvgQuatsPath));
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_CrystalStructuresArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_CrystalStructuresPath));

    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_FeatureReferenceMisorientationsArrayName_Key,
                        std::make_any<DataObjectNameParameter::ValueType>(::ConstantTest::k_GeneratedFeatureMisorientationsName));
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_FeatureAvgMisorientationsArrayName_Key,
                        std::make_any<DataObjectNameParameter::ValueType>(::ConstantTest::k_GeneratedAvgMisorientationsName));

    // Preflight the filter and check result
    auto preflightResult = filter.preflight(dataStructure, args);
    SIMPLNX_RESULT_REQUIRE_VALID(preflightResult.outputActions);

    // Execute the filter and check the result
    auto executeResult = filter.execute(dataStructure, args);
    SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result);
  }

  // Compare the Output Cell Data
  {
    UnitTest::CompareArrays<float32>(dataStructure, ::ConstantTest::k_ExemplarCellArrayPath, ::ConstantTest::k_GeneratedCellArrayPath);
  }

  // Compare the Output Feature Data
  {
    UnitTest::CompareArrays<float32>(dataStructure, ::ConstantTest::k_ExemplarCellFeatureArrayPath, ::ConstantTest::k_GeneratedCellFeatureArrayPath);
  }

#ifdef SIMPLNX_WRITE_TEST_OUTPUT
  WriteTestDataStructure(dataStructure, fs::path(fmt::format("{}/find_feat_ref_misorientations_constant_z_aligned.dream3d", unit_test::k_BinaryTestOutputDir)));
#endif
}

TEST_CASE("OrientationAnalysis::FindFeatureReferenceMisorientationsFilter: Constant Misorientation Custom Test", "[OrientationAnalysis][FindFeatureReferenceMisorientationsFilter]")
{
  const nx::core::UnitTest::TestFileSentinel testDataSentinel(nx::core::unit_test::k_CMakeExecutable, nx::core::unit_test::k_TestFilesDir, fmt::format("{}.tar.gz", ::ConstantTest::k_TestingDirName),
                                                              ::ConstantTest::k_TestingDirName);

  // Load DataStructure From Input File
  auto baseDataFilePath = fs::path(fmt::format("{}/{}/misorientation_edge_case.dream3d", unit_test::k_TestFilesDir, ::ConstantTest::k_TestingDirName));
  DataStructure dataStructure = UnitTest::LoadDataStructure(baseDataFilePath);

  // Instantiate the filter and an Arguments Object
  {
    FindFeatureReferenceMisorientationsFilter filter;
    Arguments args;

    // Create default Parameters for the filter.
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_ReferenceOrientation_Key, std::make_any<ChoicesParameter::ValueType>(2));
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_ConstantRefOrientationVec_Key, std::make_any<std::vector<float32>>(std::vector<float32>{1.0f, 1.0f, 1.0f, 45.0f}));

    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_CellFeatureIdsArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_FeatureIdsPath));
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_CellPhasesArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_PhasesPath));
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_QuatsArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_QuatsPath));

    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_AvgQuatsArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_AvgQuatsPath));
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_CrystalStructuresArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_CrystalStructuresPath));

    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_FeatureReferenceMisorientationsArrayName_Key,
                        std::make_any<DataObjectNameParameter::ValueType>(::ConstantTest::k_GeneratedFeatureMisorientationsName));
    args.insertOrAssign(FindFeatureReferenceMisorientationsFilter::k_FeatureAvgMisorientationsArrayName_Key,
                        std::make_any<DataObjectNameParameter::ValueType>(::ConstantTest::k_GeneratedAvgMisorientationsName));

    // Preflight the filter and check result
    auto preflightResult = filter.preflight(dataStructure, args);
    SIMPLNX_RESULT_REQUIRE_VALID(preflightResult.outputActions);

    // Execute the filter and check the result
    auto executeResult = filter.execute(dataStructure, args);
    SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result);
  }

  // Compare the Output Cell Data
  {
    UnitTest::CompareArrays<float32>(dataStructure, ::ConstantTest::k_ExemplarCellArrayPath, ::ConstantTest::k_GeneratedCellArrayPath);
  }

  // Compare the Output Feature Data
  {
    UnitTest::CompareArrays<float32>(dataStructure, ::ConstantTest::k_ExemplarCellFeatureArrayPath, ::ConstantTest::k_GeneratedCellFeatureArrayPath);
  }

#ifdef SIMPLNX_WRITE_TEST_OUTPUT
  WriteTestDataStructure(dataStructure, fs::path(fmt::format("{}/find_feat_ref_misorientations_constant_custom.dream3d", unit_test::k_BinaryTestOutputDir)));
#endif
}