#include "OrientationAnalysis/Filters/GenerateMisorientationColorsFilter.hpp"
#include "OrientationAnalysis/OrientationAnalysis_test_dirs.hpp"

#include "simplnx/Parameters/VectorParameter.hpp"
#include "simplnx/UnitTest/UnitTestCommon.hpp"
#include "simplnx/Utilities/Parsing/DREAM3D/Dream3dIO.hpp"
#include "simplnx/Utilities/Parsing/HDF5/Writers/FileWriter.hpp"

#include <catch2/catch.hpp>

#include <cstdio>
#include <filesystem>

namespace fs = std::filesystem;
using namespace nx::core;
using namespace nx::core::UnitTest;
using namespace nx::core::Constants;

namespace
{
// inline constexpr StringLiteral k_DataContainer("DataContainer");
// inline constexpr StringLiteral k_CellAttributeMatrix("CellData");
inline constexpr StringLiteral k_EulersCubic("Eulers_Cubic");
inline constexpr StringLiteral k_PhasesCubic("Phases_Cubic");
inline constexpr StringLiteral k_QuatsCubic("Quats_Cubic");
inline constexpr StringLiteral k_ColorsCubic("MisorientationColor_Cubic");

inline constexpr StringLiteral k_EulersHex("Eulers_Hex");
inline constexpr StringLiteral k_PhasesHex("Phases_Hex");
inline constexpr StringLiteral k_QuatsHex("Quats_Hex");
inline constexpr StringLiteral k_ColorsHex("MisorientationColor_Hex");

inline constexpr StringLiteral k_GeneratedColors("Generated Misorientation Colors");

inline constexpr StringLiteral k_OutputMisorientationColors("Misorientation Colors_Test_Output");
} // namespace

TEST_CASE("OrientationAnalysis::GenerateMisorientationColors (Cubic)", "[OrientationAnalysis][GenerateMisorientationColors]")
{
  Application::GetOrCreateInstance()->loadPlugins(unit_test::k_BuildDir.view(), true);

  const nx::core::UnitTest::TestFileSentinel testDataSentinel(nx::core::unit_test::k_CMakeExecutable, nx::core::unit_test::k_TestFilesDir, "6_6_MisorientationColors.tar.gz",
                                                              "6_6_MisorientationColors.dream3d");
  auto baseDataFilePath = fs::path(fmt::format("{}/6_6_MisorientationColors/6_6_MisorientationColors.dream3d", unit_test::k_TestFilesDir));

  DataStructure dataStructure = UnitTest::LoadDataStructure(baseDataFilePath);

  // Instantiate the filter, a DataStructure object and an Arguments Object
  {
    GenerateMisorientationColorsFilter filter;
    Arguments args;

    DataPath cellEulerAnglesPath({Constants::k_DataContainer, Constants::k_CellData, k_EulersCubic});
    DataPath cellQuatsPath({Constants::k_DataContainer, Constants::k_CellData, k_QuatsCubic});
    DataPath cellPhasesArrayPath({Constants::k_DataContainer, Constants::k_CellData, k_PhasesCubic});
    //  DataPath goodVoxelsPath({Constants::k_ImageDataContainer, Constants::k_CellData, Constants::k_Mask});
    DataPath crystalStructuresArrayPath({Constants::k_DataContainer, Constants::k_EnsembleAttributeMatrix, Constants::k_CrystalStructures});

    // Create default Parameters for the filter.
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_ReferenceAxis_Key, std::make_any<VectorFloat32Parameter::ValueType>({0.0F, 0.0F, 1.0F, 0.0F}));
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_UseMask_Key, std::make_any<bool>(false));
    // args.insertOrAssign(GenerateMisorientationColorsFilter::k_MaskArrayPath_Key, std::make_any<DataPath>(goodVoxelsPath));

    args.insertOrAssign(GenerateMisorientationColorsFilter::k_UseEulers_Key, std::make_any<bool>(false));
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_CellEulerAnglesArrayPath_Key, std::make_any<DataPath>(cellEulerAnglesPath));

    args.insertOrAssign(GenerateMisorientationColorsFilter::k_CellQuatsArrayPath_Key, std::make_any<DataPath>(cellQuatsPath));

    args.insertOrAssign(GenerateMisorientationColorsFilter::k_CellPhasesArrayPath_Key, std::make_any<DataPath>(cellPhasesArrayPath));
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_CrystalStructuresArrayPath_Key, std::make_any<DataPath>(crystalStructuresArrayPath));
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_CellMisorientationColorsArrayName_Key, std::make_any<std::string>(k_GeneratedColors));

    //   REQUIRE(dataStructure.getData(goodVoxelsPath) != nullptr);
    REQUIRE(dataStructure.getData(cellQuatsPath) != nullptr);
    REQUIRE(dataStructure.getData(cellPhasesArrayPath) != nullptr);

    // Preflight the filter and check result
    auto preflightResult = filter.preflight(dataStructure, args);
    SIMPLNX_RESULT_REQUIRE_VALID(preflightResult.outputActions);

    // Execute the filter and check the result
    auto executeResult = filter.execute(dataStructure, args);
    SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result);

#ifdef SIMPLNX_WRITE_TEST_OUTPUT
    WriteTestDataStructure(dataStructure, fs::path(fmt::format("{}/GenerateMisorientationColors_Test.dream3d", unit_test::k_BinaryTestOutputDir)));
#endif

    // compare the resulting misorientation Colors array
    DataPath exemplarPath({Constants::k_DataContainer, Constants::k_CellData, k_ColorsCubic});
    DataPath generatedPath({Constants::k_DataContainer, Constants::k_CellData, k_GeneratedColors});

    auto& exemplarRef = dataStructure.getDataRefAs<IDataArray>(exemplarPath);
    auto& generatedRef = dataStructure.getDataRefAs<IDataArray>(generatedPath);

    CompareDataArrays<uint8>(exemplarRef, generatedRef);
  }
}

TEST_CASE("OrientationAnalysis::GenerateMisorientationColors (Hex)", "[OrientationAnalysis][GenerateMisorientationColors]")
{
  Application::GetOrCreateInstance()->loadPlugins(unit_test::k_BuildDir.view(), true);

  const nx::core::UnitTest::TestFileSentinel testDataSentinel(nx::core::unit_test::k_CMakeExecutable, nx::core::unit_test::k_TestFilesDir, "6_6_MisorientationColors.tar.gz",
                                                              "6_6_MisorientationColors.dream3d");
  auto baseDataFilePath = fs::path(fmt::format("{}/6_6_MisorientationColors/6_6_MisorientationColors.dream3d", unit_test::k_TestFilesDir));

  DataStructure dataStructure = UnitTest::LoadDataStructure(baseDataFilePath);

  // Instantiate the filter, a DataStructure object and an Arguments Object
  {
    GenerateMisorientationColorsFilter filter;
    Arguments args;

    DataPath cellEulerAnglesPath({Constants::k_DataContainer, Constants::k_CellData, k_EulersHex});
    DataPath cellQuatsPath({Constants::k_DataContainer, Constants::k_CellData, k_QuatsHex});
    DataPath cellPhasesArrayPath({Constants::k_DataContainer, Constants::k_CellData, k_PhasesHex});
    //  DataPath goodVoxelsPath({Constants::k_ImageDataContainer, Constants::k_CellData, Constants::k_Mask});
    DataPath crystalStructuresArrayPath({Constants::k_DataContainer, Constants::k_EnsembleAttributeMatrix, Constants::k_CrystalStructures});

    // Create default Parameters for the filter.
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_ReferenceAxis_Key, std::make_any<VectorFloat32Parameter::ValueType>({0.0F, 0.0F, 1.0F, 0.0F}));
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_UseMask_Key, std::make_any<bool>(false));
    // args.insertOrAssign(GenerateMisorientationColorsFilter::k_MaskArrayPath_Key, std::make_any<DataPath>(goodVoxelsPath));

    args.insertOrAssign(GenerateMisorientationColorsFilter::k_UseEulers_Key, std::make_any<bool>(false));
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_CellEulerAnglesArrayPath_Key, std::make_any<DataPath>(cellEulerAnglesPath));

    args.insertOrAssign(GenerateMisorientationColorsFilter::k_CellQuatsArrayPath_Key, std::make_any<DataPath>(cellQuatsPath));

    args.insertOrAssign(GenerateMisorientationColorsFilter::k_CellPhasesArrayPath_Key, std::make_any<DataPath>(cellPhasesArrayPath));
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_CrystalStructuresArrayPath_Key, std::make_any<DataPath>(crystalStructuresArrayPath));
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_CellMisorientationColorsArrayName_Key, std::make_any<std::string>(k_GeneratedColors));

    //   REQUIRE(dataStructure.getData(goodVoxelsPath) != nullptr);
    REQUIRE(dataStructure.getData(cellQuatsPath) != nullptr);
    REQUIRE(dataStructure.getData(cellPhasesArrayPath) != nullptr);

    // Preflight the filter and check result
    auto preflightResult = filter.preflight(dataStructure, args);
    SIMPLNX_RESULT_REQUIRE_VALID(preflightResult.outputActions);

    // Execute the filter and check the result
    auto executeResult = filter.execute(dataStructure, args);
    SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result);

#ifdef SIMPLNX_WRITE_TEST_OUTPUT
    WriteTestDataStructure(dataStructure, fs::path(fmt::format("{}/GenerateMisorientationColors_Test.dream3d", unit_test::k_BinaryTestOutputDir)));
#endif

    // compare the resulting misorientation Colors array
    DataPath exemplarPath({Constants::k_DataContainer, Constants::k_CellData, k_ColorsHex});
    DataPath generatedPath({Constants::k_DataContainer, Constants::k_CellData, k_GeneratedColors});

    auto& exemplarRef = dataStructure.getDataRefAs<IDataArray>(exemplarPath);
    auto& generatedRef = dataStructure.getDataRefAs<IDataArray>(generatedPath);

    CompareDataArrays<uint8>(exemplarRef, generatedRef);
  }
}
