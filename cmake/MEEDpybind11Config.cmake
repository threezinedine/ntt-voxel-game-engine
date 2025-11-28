if (TARGET MEEDpybind11)
    return()
endif()

set(MEEDpybind11_DIR ${EXTERNALS_DIR}/pybind11)
add_subdirectory(${MEEDpybind11_DIR} ${CMAKE_BINARY_DIR}/externals/pybind11)

find_package(Python3 REQUIRED)

add_library(MEEDpybind11 INTERFACE)
target_link_libraries(
    MEEDpybind11
    INTERFACE
    pybind11::pybind11
)