set(CPM_DOWNLOAD_VERSION 0.43.1)
set(CPM_HASH_SUM "1c40fc102ce9625d7de7eb14f541cab30cc3138dca627f0b0ec40293ce6c2934")

if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    file(
        DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
        "${CMAKE_CURRENT_LIST_DIR}/CPM_${CPM_DOWNLOAD_VERSION}.cmake"
        EXPECTED_HASH SHA256=${CPM_HASH_SUM}
        TLS_VERIFY ON
    )
endif()

include("${CMAKE_CURRENT_LIST_DIR}/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
