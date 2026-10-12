cmake_minimum_required(VERSION 3.14)
project(nur VERSION 1.2.1 LANGUAGES C CXX)

# Header-only interface library target
add_library(nur INTERFACE)
add_library(nur::nur ALIAS nur)

target_include_directories(nur INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Standard GNU install directories
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

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
install(TARGETS nur EXPORT nurTargets)
install(FILES src/nur.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

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
endif()
