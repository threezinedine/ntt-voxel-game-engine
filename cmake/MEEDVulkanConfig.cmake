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

macro(CompileVulkanShader)
    set(options )
    set(oneValueArgs TARGET SHADER_FILE OUTPUT_FILE)
    set(multiValueArgs )
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_custom_command(
        TARGET ${ARG_TARGET}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${ARG_SHADER_FILE} -o ${ARG_OUTPUT_FILE}
        DEPENDS ${ARG_SHADER_FILE}
        COMMENT "Compiling Vulkan shader: ${ARG_SHADER_FILE} -> ${ARG_OUTPUT_FILE}"
    )
endmacro()
