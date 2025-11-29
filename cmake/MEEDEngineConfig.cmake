if (TARGET MEEDEngine)
    return()
endif()

set(MEEDEngine_DIR "${PROJECT_BASE_DIR}/engine")
add_subdirectory("${MEEDEngine_DIR}" "${CMAKE_BINARY_DIR}/packages/MEEDEngine")