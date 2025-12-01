cmake_minimum_required(VERSION 3.20)
project(MEEDEngine)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(MEEDConfigure)

MEEDSetup()

file(
    GLOB 
    SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/**/**/*.c
)

file(
    GLOB 
    HEADER_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MEEDEngine/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MEEDEngine/**/*.h
)

if (NOT PLATFORM_IS_WEB)
    MEEDFindPackage(MEEDpybind11 "Dependencies")
endif()

set(PROJECT_SOURCE_FILES ${SRC_FILES} ${HEADER_FILES})
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PROJECT_DEFINITIONS ${COMMON_DEFINITIONS})
set(PROJECT_PRECOMPILED_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/MEEDEngine/platforms/common.h)
set(PROJECT_LIBRARIES)

if (PLATFORM_IS_LINUX AND NOT MEED_USE_OPENGL)
    list(APPEND PROJECT_LIBRARIES X11 -rdynamic)
endif()

# ================ Engine Lib ================
add_library(
    ${PROJECT_NAME}
    STATIC
    ${PROJECT_SOURCE_FILES}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_INCLUDE_DIR}
)

target_compile_definitions(
    ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_DEFINITIONS}
)

target_precompile_headers(
    ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_PRECOMPILED_HEADER}
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_LIBRARIES}
)

if (MEED_USE_VULKAN)
    MEEDFindPackage(MEEDVulkan "Dependencies")
    target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC
        MEEDVulkan
    )

    # ============== Vulkan Shader Compilation ==============
    file(
        GLOB 
        VULKAN_SHADER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/vulkan/*.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/vulkan/*.frag
    )

    foreach(SHADER_FILE ${VULKAN_SHADER_FILES})
        get_filename_component(SHADER_NAME ${SHADER_FILE} NAME_WE)
        get_filename_component(SHADER_EXT ${SHADER_FILE} EXT)
        set(OUTPUT_FILE ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}${SHADER_EXT}.spv)
        string(REPLACE "." "" SHADER_EXT ${SHADER_EXT}) # Remove dot from extension

        CompileVulkanShader(
            SHADER_FILE ${SHADER_FILE}
            OUTPUT_FILE ${OUTPUT_FILE}
        )
    endforeach()
    CompileAllVulkanShaders(${PROJECT_NAME})
endif()

if (MEED_USE_OPENGL)
    MEEDFindPackage(MEEDGlfw "Dependencies")
    target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC
        MEEDGlfw
    )
endif()

if (NOT PLATFORM_IS_WEB)
    # ================ Examples ================
    add_subdirectory("examples") # no need to build examples for web_build

    # ================ Tests ================
    add_subdirectory("tests")

    if (MEED_DEBUG)
        # ================ Engine Binding Lib ================
        set(BINDING_PROJECT "${PROJECT_NAME}Binding")
        set(BINDING_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/tmp/binding.cpp)

        pybind11_add_module(
            ${BINDING_PROJECT}
            ${BINDING_SOURCE_FILES} 
        )

        target_link_libraries(
            ${BINDING_PROJECT}
            PUBLIC
            ${PROJECT_NAME}
            PRIVATE
            MEEDpybind11
        )

        set_target_properties(
            ${BINDING_PROJECT}
            PROPERTIES
            OUTPUT_NAME ${PROJECT_NAME}
            SUFFIX ".so"
        )

        # ================ Copy Resources ================
        set(LIB_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.so)
        set(OUTPUT_FILE ${PROJECT_BASE_DIR}/editor/Engine/${PROJECT_NAME}.so)

        add_custom_command(
            TARGET ${BINDING_PROJECT} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${LIB_FILE}
            ${OUTPUT_FILE}
            DEPENDS ${LIB_FILE}
            COMMENT "Copying MEED Engine binding to Editor directory"
        )
    endif()
endif()