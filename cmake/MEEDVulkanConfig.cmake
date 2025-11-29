if (TARGET MEEDVulkan)
    return()
endif()

find_package(Vulkan REQUIRED)

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