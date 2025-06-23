cmake_minimum_required(VERSION 3.14)
project(RustCxx VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Header-only library
add_library(rustcxx INTERFACE)
target_include_directories(
    rustcxx
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Example executable
option(RUSTCXX_BUILD_EXAMPLES "Build examples" ON)

if(RUSTCXX_BUILD_EXAMPLES)
    add_executable(rustcxx_basic_usage examples/basic_usage.cpp)
    target_link_libraries(rustcxx_basic_usage rustcxx)

    # Set compiler warnings
    if(MSVC)
        target_compile_options(rustcxx_basic_usage PRIVATE /W4)
    else()
        target_compile_options(
            rustcxx_basic_usage
            PRIVATE -Wall -Wextra -Wpedantic
        )
    endif()
endif()

# Testing
option(RUSTCXX_BUILD_TESTS "Build tests" ON)

if(RUSTCXX_BUILD_TESTS)
    enable_testing()

    # Find or fetch Google Test
    find_package(GTest QUIET)

    add_executable(
        rustcxx_tests
        tests/test_enum.cpp
        tests/test_result.cpp
        tests/test_option.cpp
    )
    target_link_libraries(rustcxx_tests rustcxx gtest gtest_main)

    # Set compiler warnings for tests
    if(MSVC)
        target_compile_options(rustcxx_tests PRIVATE /W4)
    else()
        target_compile_options(rustcxx_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME rustcxx_unit_tests COMMAND rustcxx_tests)
endif()

# Installation
include(GNUInstallDirs)

install(
    TARGETS rustcxx
    EXPORT rustcxxTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
    EXPORT rustcxxTargets
    FILE rustcxxTargets.cmake
    NAMESPACE rustcxx::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rustcxx
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/rustcxxConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/rustcxxConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rustcxx
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/rustcxxConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/rustcxxConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/rustcxxConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rustcxx
)
