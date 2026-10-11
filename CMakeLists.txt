cmake_minimum_required(VERSION 3.14)
project(nur.h VERSION 1.0.0 LANGUAGES C CXX)

# Header-only interface library
add_library(nur INTERFACE)
add_library(nur::nur ALIAS nur)

target_include_directories(nur INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

# Installation specification
include(GNUInstallDirs)
install(TARGETS nur EXPORT nurTargets)
install(FILES src/nur.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT nurTargets
    FILE nurTargets.cmake
    NAMESPACE nur::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nur
)

# Optional testing
option(NUR_BUILD_TESTS "Build nur.h test suite" ON)
if(NUR_BUILD_TESTS)
    enable_testing()
    add_executable(test_nur test/test_nur.c)
    target_link_libraries(test_nur PRIVATE nur)
    target_compile_options(test_nur PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
        $<$<C_COMPILER_ID:MSVC>:/W4>
    )
    add_test(NAME test_nur COMMAND test_nur)
endif()
