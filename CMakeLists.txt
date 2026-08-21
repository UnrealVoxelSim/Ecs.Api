cmake_minimum_required(VERSION 3.28)

project(UnrealVoxelSim.Ecs.Api VERSION 0.1.0 LANGUAGES CXX)

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME UnrealVoxelSimBuild
    GITHUB_REPOSITORY UnrealVoxelSim/Build.CMake
    GIT_TAG main
)

include("${UnrealVoxelSimBuild_SOURCE_DIR}/UnrealVoxelSim.Build.CMake.cmake")

UnrealVoxelSimConfigureModule()
