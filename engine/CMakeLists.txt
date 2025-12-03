cmake_minimum_required(VERSION 3.20)
project(MEEDEngine)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(MEEDConfigure)

MdSetup()

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

if (PLATFORM_IS_LINUX AND NOT MD_USE_OPENGL)
    list(APPEND PROJECT_LIBRARIES X11)
endif()

if (PLATFORM_IS_LINUX)
    list(APPEND PROJECT_LIBRARIES -rdynamic)
endif()

set(CMAKE_FOLDER "MEED")
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

target_link_options(
    ${PROJECT_NAME}
    PUBLIC
    ${COMMON_OPTIONS}
)

if (PLATFORM_IS_WEB)
    target_link_options(
        ${PROJECT_NAME}
        PUBLIC
        -sUSE_GLFW=3
        -sUSE_WEBGL2=1
        -sFULL_ES3=1
    )
endif()

if (MD_USE_VULKAN)
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

if (MD_USE_OPENGL)
    MEEDFindPackage(MEEDGlfw "Dependencies")
    target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC
        MEEDGlfw
    )
endif()
unset(CMAKE_FOLDER)

if (NOT PLATFORM_IS_WEB)
    # ================ Examples ================
    add_subdirectory("examples") # no need to build examples for web_build

    set(CMAKE_FOLDER "MEEDTests")
    # ================ Tests ================
    add_subdirectory("tests")
    unset(CMAKE_FOLDER)

    set(CMAKE_FOLDER "MEEDBindings")
    if (MD_DEBUG)
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

        if (PLATFORM_IS_LINUX)
            set_target_properties(
                ${BINDING_PROJECT}
                PROPERTIES
                OUTPUT_NAME ${PROJECT_NAME}
                SUFFIX ".so"
            )
        elseif (PLATFORM_IS_WINDOWS)
            set_target_properties(
                ${BINDING_PROJECT}
                PROPERTIES
                OUTPUT_NAME ${PROJECT_NAME}Binding
                SUFFIX ".pyd"
            )
        endif()

        # ================ Copy Resources ================
        if (PLATFORM_IS_LINUX)
            set(BINDING_OUTPUT_NAME ${PROJECT_NAME}.so)
        elseif (PLATFORM_IS_WINDOWS)
            if (MD_DEBUG)
                set(BINDING_OUTPUT_NAME Debug/${PROJECT_NAME}Binding.pyd)
            else()
                set(BINDING_OUTPUT_NAME Release/${PROJECT_NAME}Binding.pyd)
            endif()
        endif()

        set(LIB_FILE ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_OUTPUT_NAME})
        set(OUTPUT_FILE ${PROJECT_BASE_DIR}/editor/Engine/${BINDING_OUTPUT_NAME})

        add_custom_command(
            TARGET ${BINDING_PROJECT} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${LIB_FILE}
            ${OUTPUT_FILE}
            DEPENDS ${LIB_FILE}
            COMMENT "Copying MEED Engine binding to Editor directory"
        )
    endif()
    unset(CMAKE_FOLDER)
endif()