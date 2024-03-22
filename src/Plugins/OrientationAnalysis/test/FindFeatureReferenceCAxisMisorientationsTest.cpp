#include <catch2/catch.hpp>

#include "simplnx/UnitTest/UnitTestCommon.hpp"

#include "OrientationAnalysis/Filters/FindFeatureReferenceCAxisMisorientationsFilter.hpp"
#include "OrientationAnalysis/OrientationAnalysis_test_dirs.hpp"

using namespace nx::core;
using namespace nx::core::Constants;

namespace
{
const std::string k_FeatRefCAxisMisExemplar = "FeatureReferenceCAxisMisorientations";
const std::string k_FeatRefCAxisMisComputed = "NX_FeatureReferenceCAxisMisorientations";
const std::string k_FeatAvgCAxisMisExemplar = "FeatureAvgCAxisMisorientations";
const std::string k_FeatAvgCAxisMisComputed = "NX_FeatureAvgCAxisMisorientations";
const std::string k_FeatStDevCAxisMisExemplar = "FeatureStdevCAxisMisorientations";
const std::string k_FeatStDevCAxisMisComputed = "NX_FeatureStdevCAxisMisorientations";
const DataPath k_AvgCAxesPath = k_CellFeatureDataPath.createChildPath("AvgCAxes");

namespace ConstantTest
{
const std::string k_TestingDirName("find_feature_reference_c_axis_misorientations_test");

const DataPath k_ImageGeomPath({"Misorientation Test"});

// Ensemble Attribute Matrix
const DataPath k_CrystalStructuresPath = k_ImageGeomPath.createChildPath(Constants::k_EnsembleAttributeMatrix).createChildPath(Constants::k_CrystalStructures);

// Cell Data
const DataPath k_CellData = k_ImageGeomPath.createChildPath("Cell Data");

const DataPath k_PhasesPath = k_CellData.createChildPath("phases");
const DataPath k_FeatureIdsPath = k_CellData.createChildPath("featureIds");

const std::string k_GeneratedFeatureMisorientationsName("GeneratedFeatureReferenceMisorientations");
const DataPath k_GeneratedCellArrayPath = k_CellData.createChildPath(k_GeneratedFeatureMisorientationsName);
const DataPath k_ExemplarCellArrayPath = k_CellData.createChildPath("ExemplarFeatureReferenceMisorientations");

// Cell Feature Data
const DataPath k_CellFeatureData = k_ImageGeomPath.createChildPath("Cell Feature Data");

const DataPath k_AvgCAxisPath = k_CellFeatureData.createChildPath("AvgCAxis");

const std::string k_GeneratedAvgMisorientationsName("GeneratedFeatureAvgMisorientations");
const DataPath k_GeneratedCellFeatureArrayPath = k_CellFeatureData.createChildPath(k_GeneratedAvgMisorientationsName);
const DataPath k_ExemplarCellFeatureArrayPath = k_CellFeatureData.createChildPath("ExemplarFeatureAvgMisorientations");
} // namespace ConstantTest
} // namespace

TEST_CASE("OrientationAnalysis::FindFeatureReferenceCAxisMisorientationsFilter: Valid Filter Execution", "[OrientationAnalysis][FindFeatureReferenceCAxisMisorientationsFilter]")
{
  Application::GetOrCreateInstance()->loadPlugins(unit_test::k_BuildDir.view(), true);

  const nx::core::UnitTest::TestFileSentinel testDataSentinel(nx::core::unit_test::k_CMakeExecutable, nx::core::unit_test::k_TestFilesDir, "6_6_caxis_data.tar.gz", "6_6_caxis_data");

  // Read Exemplar DREAM3D File Filter
  auto exemplarFilePath = fs::path(fmt::format("{}/6_6_caxis_data/6_6_find_caxis_data.dream3d", unit_test::k_TestFilesDir));
  DataStructure dataStructure = UnitTest::LoadDataStructure(exemplarFilePath);

  // Instantiate the filter, a DataStructure object and an Arguments Object
  FindFeatureReferenceCAxisMisorientationsFilter filter;
  Arguments args;

  // Create default Parameters for the filter.
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_ImageGeometryPath_Key, std::make_any<DataPath>(k_DataContainerPath));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_FeatureIdsArrayPath_Key, std::make_any<DataPath>(k_FeatureIdsArrayPath));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_CellPhasesArrayPath_Key, std::make_any<DataPath>(k_PhasesArrayPath));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_QuatsArrayPath_Key, std::make_any<DataPath>(k_QuatsArrayPath));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_AvgCAxesArrayPath_Key, std::make_any<DataPath>(k_AvgCAxesPath));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_CrystalStructuresArrayPath_Key, std::make_any<DataPath>(k_CrystalStructuresArrayPath));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_FeatureAvgCAxisMisorientationsArrayName_Key, std::make_any<std::string>(k_FeatAvgCAxisMisComputed));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_FeatureStdevCAxisMisorientationsArrayName_Key, std::make_any<std::string>(k_FeatStDevCAxisMisComputed));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_FeatureReferenceCAxisMisorientationsArrayName_Key, std::make_any<std::string>(k_FeatRefCAxisMisComputed));

  // Preflight the filter and check result
  auto preflightResult = filter.preflight(dataStructure, args);
  SIMPLNX_RESULT_REQUIRE_VALID(preflightResult.outputActions)

  // Execute the filter and check the result
  auto executeResult = filter.execute(dataStructure, args);
  SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result)

  UnitTest::CompareFloatArraysWithNans<float32>(dataStructure, k_CellAttributeMatrix.createChildPath(k_FeatRefCAxisMisExemplar), k_CellAttributeMatrix.createChildPath(k_FeatRefCAxisMisComputed),
                                                UnitTest::EPSILON, false);
  UnitTest::CompareFloatArraysWithNans<float32>(dataStructure, k_CellFeatureDataPath.createChildPath(k_FeatAvgCAxisMisExemplar), k_CellFeatureDataPath.createChildPath(k_FeatAvgCAxisMisComputed),
                                                UnitTest::EPSILON, false);
  UnitTest::CompareFloatArraysWithNans<float32>(dataStructure, k_CellFeatureDataPath.createChildPath(k_FeatStDevCAxisMisExemplar), k_CellFeatureDataPath.createChildPath(k_FeatStDevCAxisMisComputed),
                                                UnitTest::EPSILON, false);
}

TEST_CASE("OrientationAnalysis::FindFeatureReferenceCAxisMisorientationsFilter: InValid Filter Execution", "[OrientationAnalysis][FindFeatureReferenceCAxisMisorientationsFilter]")
{
  Application::GetOrCreateInstance()->loadPlugins(unit_test::k_BuildDir.view(), true);

  const nx::core::UnitTest::TestFileSentinel testDataSentinel(nx::core::unit_test::k_CMakeExecutable, nx::core::unit_test::k_TestFilesDir, "6_6_caxis_data.tar.gz", "6_6_caxis_data");

  // Read Exemplar DREAM3D File Filter
  auto exemplarFilePath = fs::path(fmt::format("{}/6_6_caxis_data/6_6_find_caxis_data.dream3d", unit_test::k_TestFilesDir));
  DataStructure dataStructure = UnitTest::LoadDataStructure(exemplarFilePath);

  auto& crystalStructs = dataStructure.getDataRefAs<UInt32Array>(k_CrystalStructuresArrayPath);
  crystalStructs[1] = 1;

  // Instantiate the filter, a DataStructure object and an Arguments Object
  FindFeatureReferenceCAxisMisorientationsFilter filter;
  Arguments args;

  // Invalid crystal structure type : should fail in execute
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_ImageGeometryPath_Key, std::make_any<DataPath>(k_DataContainerPath));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_FeatureIdsArrayPath_Key, std::make_any<DataPath>(k_FeatureIdsArrayPath));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_CellPhasesArrayPath_Key, std::make_any<DataPath>(k_PhasesArrayPath));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_QuatsArrayPath_Key, std::make_any<DataPath>(k_QuatsArrayPath));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_AvgCAxesArrayPath_Key, std::make_any<DataPath>(k_AvgCAxesPath));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_CrystalStructuresArrayPath_Key, std::make_any<DataPath>(k_CrystalStructuresArrayPath));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_FeatureAvgCAxisMisorientationsArrayName_Key, std::make_any<std::string>(k_FeatAvgCAxisMisComputed));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_FeatureStdevCAxisMisorientationsArrayName_Key, std::make_any<std::string>(k_FeatStDevCAxisMisComputed));
  args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_FeatureReferenceCAxisMisorientationsArrayName_Key, std::make_any<std::string>(k_FeatRefCAxisMisComputed));

  // Preflight the filter and check result
  auto preflightResult = filter.preflight(dataStructure, args);
  SIMPLNX_RESULT_REQUIRE_VALID(preflightResult.outputActions)

  // Execute the filter and check the result
  auto executeResult = filter.execute(dataStructure, args);
  SIMPLNX_RESULT_REQUIRE_INVALID(executeResult.result)
}

TEST_CASE("OrientationAnalysis::FindFeatureReferenceCAxisMisorientationsFilter: Constant Misorientation Z-Aligned Test", "[OrientationAnalysis][FindFeatureReferenceCAxisMisorientationsFilter]")
{
  const nx::core::UnitTest::TestFileSentinel testDataSentinel(nx::core::unit_test::k_CMakeExecutable, nx::core::unit_test::k_TestFilesDir, fmt::format("{}.tar.gz", ::ConstantTest::k_TestingDirName),
                                                              ::ConstantTest::k_TestingDirName);

  // Load DataStructure From Input File
  auto baseDataFilePath = fs::path(fmt::format("{}/{}/c_axis_misorientation_test_z_aligned.dream3d", unit_test::k_TestFilesDir, ::ConstantTest::k_TestingDirName));
  DataStructure dataStructure = UnitTest::LoadDataStructure(baseDataFilePath);

  // Instantiate the filter and an Arguments Object
  {
    FindFeatureReferenceCAxisMisorientationsFilter filter;
    Arguments args;

    // Create default Parameters for the filter.
    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_UseConstantReferenceOrientation_Key, std::make_any<bool>(true));
    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_ConstantRefOrientationVec_Key, std::make_any<std::vector<float32>>(std::vector<float32>{0.0f, 0.0f, 1.0f, 0.0f}));

    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_FeatureIdsArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_FeatureIdsPath));
    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_CellPhasesArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_PhasesPath));

    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_AvgCAxesArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_AvgCAxisPath));
    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_CrystalStructuresArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_CrystalStructuresPath));

    //    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_FeatureReferenceMisorientationsArrayName_Key,
    //                        std::make_any<DataObjectNameParameter::ValueType>(::ConstantTest::k_GeneratedFeatureMisorientationsName));
    //    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_FeatureAvgMisorientationsArrayName_Key,
    //                        std::make_any<DataObjectNameParameter::ValueType>(::ConstantTest::k_GeneratedAvgMisorientationsName));

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
  WriteTestDataStructure(dataStructure, fs::path(fmt::format("{}/find_feat_ref_c_axis_misorientations_constant_z_aligned.dream3d", unit_test::k_BinaryTestOutputDir)));
#endif
}

TEST_CASE("OrientationAnalysis::FindFeatureReferenceCAxisMisorientationsFilter: Constant Misorientation Custom Test", "[OrientationAnalysis][FindFeatureReferenceCAxisMisorientationsFilter]")
{
  const nx::core::UnitTest::TestFileSentinel testDataSentinel(nx::core::unit_test::k_CMakeExecutable, nx::core::unit_test::k_TestFilesDir, fmt::format("{}.tar.gz", ::ConstantTest::k_TestingDirName),
                                                              ::ConstantTest::k_TestingDirName);

  // Load DataStructure From Input File
  auto baseDataFilePath = fs::path(fmt::format("{}/{}/c_axis_misorientation_test_edge_case.dream3d", unit_test::k_TestFilesDir, ::ConstantTest::k_TestingDirName));
  DataStructure dataStructure = UnitTest::LoadDataStructure(baseDataFilePath);

  // Instantiate the filter and an Arguments Object
  {
    FindFeatureReferenceCAxisMisorientationsFilter filter;
    Arguments args;

    // Create default Parameters for the filter.
    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_UseConstantReferenceOrientation_Key, std::make_any<bool>(true));
    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_ConstantRefOrientationVec_Key, std::make_any<std::vector<float32>>(std::vector<float32>{1.0f, 1.0f, 1.0f, 45.0f}));

    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_FeatureIdsArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_FeatureIdsPath));
    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_CellPhasesArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_PhasesPath));

    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_AvgCAxesArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_AvgCAxisPath));
    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_CrystalStructuresArrayPath_Key, std::make_any<DataPath>(::ConstantTest::k_CrystalStructuresPath));

    //    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_FeatureReferenceMisorientationsArrayName_Key,
    //                        std::make_any<DataObjectNameParameter::ValueType>(::ConstantTest::k_GeneratedFeatureMisorientationsName));
    //    args.insertOrAssign(FindFeatureReferenceCAxisMisorientationsFilter::k_FeatureAvgMisorientationsArrayName_Key,
    //                        std::make_any<DataObjectNameParameter::ValueType>(::ConstantTest::k_GeneratedAvgMisorientationsName));

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
  WriteTestDataStructure(dataStructure, fs::path(fmt::format("{}/find_feat_ref_c_axis_misorientations_constant_custom.dream3d", unit_test::k_BinaryTestOutputDir)));
#endif
}