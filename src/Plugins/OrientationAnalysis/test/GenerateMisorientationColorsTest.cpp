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

namespace nx::core::Constants
{
inline constexpr StringLiteral k_ImageDataContainer("ImageDataContainer");
inline constexpr StringLiteral k_OutputMisorientationColors("Misorientation Colors_Test_Output");
} // namespace nx::core::Constants

TEST_CASE("OrientationAnalysis::GenerateMisorientationColors", "[OrientationAnalysis][GenerateMisorientationColors]")
{
  Application::GetOrCreateInstance()->loadPlugins(unit_test::k_BuildDir.view(), true);

  const nx::core::UnitTest::TestFileSentinel testDataSentinel(nx::core::unit_test::k_CMakeExecutable, nx::core::unit_test::k_TestFilesDir, "so3_cubic_high_Misorientation_001.tar.gz",
                                                              "so3_cubic_high_Misorientation_001.dream3d");

  DataStructure dataStructure;
  {

    // This test file was produced by SIMPL/DREAM3D. our results should match theirs
    auto exemplarFilePath = fs::path(fmt::format("{}/so3_cubic_high_Misorientation_001.dream3d", unit_test::k_TestFilesDir));
    REQUIRE(fs::exists(exemplarFilePath));
    auto result = DREAM3D::ImportDataStructureFromFile(exemplarFilePath);
    REQUIRE(result.valid());
    dataStructure = result.value();
  }

  // Instantiate the filter, a DataStructure object and an Arguments Object
  {
    GenerateMisorientationColorsFilter filter;
    Arguments args;

    DataPath cellEulerAnglesPath({Constants::k_ImageDataContainer, Constants::k_CellData, Constants::k_EulerAngles});
    DataPath cellPhasesArrayPath({Constants::k_ImageDataContainer, Constants::k_CellData, Constants::k_Phases});
    DataPath goodVoxelsPath({Constants::k_ImageDataContainer, Constants::k_CellData, Constants::k_Mask});
    DataPath crystalStructuresArrayPath({Constants::k_ImageDataContainer, Constants::k_CellEnsembleData, Constants::k_CrystalStructures});
    DataPath cellMisorientationColorsArrayName({Constants::k_ImageDataContainer, Constants::k_CellData, Constants::k_OutputMisorientationColors});

    // Create default Parameters for the filter.
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_ReferenceAxis_Key, std::make_any<VectorFloat32Parameter::ValueType>({0.0F, 0.0F, 1.0F, 0.0F}));
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_UseMask_Key, std::make_any<bool>(true));
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_CellEulerAnglesArrayPath_Key, std::make_any<DataPath>(cellEulerAnglesPath));
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_CellPhasesArrayPath_Key, std::make_any<DataPath>(cellPhasesArrayPath));
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_MaskArrayPath_Key, std::make_any<DataPath>(goodVoxelsPath));
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_CrystalStructuresArrayPath_Key, std::make_any<DataPath>(crystalStructuresArrayPath));
    args.insertOrAssign(GenerateMisorientationColorsFilter::k_CellMisorientationColorsArrayName_Key, std::make_any<std::string>(Constants::k_OutputMisorientationColors));

    REQUIRE(dataStructure.getData(goodVoxelsPath) != nullptr);
    REQUIRE(dataStructure.getData(cellEulerAnglesPath) != nullptr);
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

    DataPath MisorientationColors({Constants::k_ImageDataContainer, Constants::k_CellData, "name+here"});

    auto& exemplar = dataStructure.getDataRefAs<UInt8Array>(MisorientationColors);
    auto& output = dataStructure.getDataRefAs<UInt8Array>(cellMisorientationColorsArrayName);

    size_t totalElements = exemplar.getSize();
    bool valid = true;
    for(size_t i = 0; i < totalElements; i++)
    {
      if(exemplar[i] != output[i])
      {
        valid = false;
        break;
      }
    }
    REQUIRE(valid == true);
  }
}
