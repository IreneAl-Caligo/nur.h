cmake_minimum_required(VERSION 3.14)
project(nur VERSION 1.4.0 LANGUAGES C CXX)

# Standard GNU install directories & package helpers
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Check if nur is the top-level project or included via FetchContent / add_subdirectory
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(NUR_IS_TOP_LEVEL_PROJECT ON)
else()
    set(NUR_IS_TOP_LEVEL_PROJECT OFF)
endif()

# Header-only interface library target: nur (full batteries-included)
add_library(nur INTERFACE)
add_library(nur::nur ALIAS nur)

target_include_directories(nur INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Header-only interface library target: nur_core (ultra-lean primitives)
add_library(nur_core INTERFACE)
add_library(nur::core ALIAS nur_core)

target_include_directories(nur_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Generate Package Version File
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/nurConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Generate Package Config File
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/nurConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/nurConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/nur"
)

# Installation specification
install(TARGETS nur nur_core EXPORT nurTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(FILES src/nur.h src/nur_core.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install CMake package config files
install(EXPORT nurTargets
    FILE nurTargets.cmake
    NAMESPACE nur::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nur
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/nurConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/nurConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nur
)

# Install pkg-config file
install(FILES nur.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# Optional testing (defaults to OFF when included via FetchContent)
option(NUR_BUILD_TESTS "Build nur.h test suite" ${NUR_IS_TOP_LEVEL_PROJECT})

if(NUR_BUILD_TESTS)
    enable_testing()

    add_executable(test_nur test/test_nur.c)
    target_link_libraries(test_nur PRIVATE nur)
    target_compile_options(test_nur PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
        $<$<C_COMPILER_ID:MSVC>:/W4>
    )
    add_test(NAME test_nur COMMAND test_nur)

    add_executable(test_universal_type test/test_universal_type.c)
    target_link_libraries(test_universal_type PRIVATE nur)
    target_compile_options(test_universal_type PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
        $<$<C_COMPILER_ID:MSVC>:/W4>
    )
    add_test(NAME test_universal_type COMMAND test_universal_type)

    add_executable(test_all_features test/test_all_features_and_errors.c)
    target_link_libraries(test_all_features PRIVATE nur)
    target_compile_options(test_all_features PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
        $<$<C_COMPILER_ID:MSVC>:/W4>
    )
    add_test(NAME test_all_features COMMAND test_all_features)

    add_executable(test_abi_layout test/test_abi_layout.c)
    target_link_libraries(test_abi_layout PRIVATE nur)
    target_compile_options(test_abi_layout PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
        $<$<C_COMPILER_ID:MSVC>:/W4>
    )
    add_test(NAME test_abi_layout COMMAND test_abi_layout)
endif()
