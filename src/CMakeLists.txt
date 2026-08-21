add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/public>"
        "$<INSTALL_INTERFACE:include>"
)

UnrealVoxelSimConfigureTarget(${PROJECT_NAME})
