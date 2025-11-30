if (TARGET MEEDVulkan)
    return()
endif()

find_package(Vulkan REQUIRED COMPONENTS glslc)

add_library(
    MEEDVulkan 
    INTERFACE
)

target_include_directories(
    MEEDVulkan 
    INTERFACE 
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(
    MEEDVulkan 
    INTERFACE 
    ${Vulkan_LIBRARIES}
)

message(STATUS "Found Vulkan SDK: ${Vulkan_VERSION}")

set(ALL_SPV_FILES)

macro(CompileVulkanShader)
    set(options )
    set(oneValueArgs SHADER_FILE OUTPUT_FILE)
    set(multiValueArgs )
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_custom_command(
        OUTPUT ${ARG_OUTPUT_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${ARG_SHADER_FILE} -o ${ARG_OUTPUT_FILE}
        DEPENDS ${ARG_SHADER_FILE}
        COMMENT "Compiling Vulkan shader: ${ARG_SHADER_FILE} -> ${ARG_OUTPUT_FILE}"
    )
    list(APPEND ALL_SPV_FILES ${ARG_OUTPUT_FILE})
endmacro()

macro(CompileAllVulkanShaders TARGET_NAME)
    add_custom_target(CompileAllVulkanShaders ALL
        DEPENDS ${ALL_SPV_FILES}
    )
    add_dependencies(${TARGET_NAME} CompileAllVulkanShaders)
endmacro()

