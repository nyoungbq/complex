Release Notes 24.11.11
======================

The `simplnx` library is under activate development and while we strive to maintain a stable API bugs are
found that necessitate the changing of the API.

Version 24.11.11
-----------------


API Changes & Additions 24.11.11
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Filters now have a version number that will be written to the pipeline file.

Change Log 24.11.11
^^^^^^^^^^^^^^^^^^^^

- REL: Update version to 1.5.0 (#1128) [2024-11-11]
- FILT: Add Oxford Channel 5 Binary file reader. (#1126) [2024-11-11]
- BUG: Now catch std::filesystem exceptions in parameters (#1127) [2024-11-11]
- DOC: Document the DREAM3D file specification (#1116) [2024-11-07]
- API: EbsdLib-1.0.34 update. (#1122) [2024-11-05]
- BUG: ReadDEFORMFileFilter-Set SharedQuadList names to proper defaults (#1120) [2024-11-04]
- ENH: Fix 3rd Party .dream3d file import and other compiler warnings & issues (#1108) [2024-11-04]
- LICENSE: Change the distribution license to AGPLv3 (#1119) [2024-10-29]
- ENH: Compute NeighborList Stats Updates (#1118) [2024-10-25]
- ENH: Map Point Cloud to Regular Grid Filter Modernization (#1109) [2024-10-22]
- BUG: WriteASCIIDataFilter-Fix bug writing multi-component arrays to multiple files (#1114) [2024-10-22]
- ENH: GitHub is removing MacOS-12 Support from their Actions. Update to MacOS-13 (#1115) [2024-10-21]
- ENH: Changes to decrease compile time (#1105) [2024-10-18]
- FILT: Add 'Read GrainMapper3D' filter (#1107) [2024-10-17]
- ENH: ITKImportImageStack Z Axis Resampling and Improvements (#1104) [2024-10-16]
- ENH: Iterative Closest Point Filter Changed to 1 Second Update Scheme (#1106) [2024-10-16]
- BUG FIX: Data object parent ids are now renumbered correctly when object id clashes are found. (#1102) [2024-10-10]
- BUG FIX: PeregrineHDF5Reader now properly flips images across the X axis. (#1103) [2024-10-10]
- FILT: ReshapeDataArrayFilter (#1097) [2024-10-09]
- COMP: Update clang format standard to C++20 (#1095) [2024-10-04]
- DOC: Updates to ConvertOrientationFilter documentation (#1096) [2024-10-04]
- REL: DREAM3D-NX Version 7.0.0-RC14 Release (#1089) [2024-10-01]
