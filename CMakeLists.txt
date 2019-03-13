# example CMake conf from https://github.com/forexample/package-example

# Usage:
#   mkdir build && cd build && cmake ..
#
# This file will export module 'libfronthaul' into the user package registry (~/.cmake/packages)
# Then you should be to compile against this library with ease; no need to do "make install"'s,
# just add these into your project:
#
#   find_package(libfronthaul 0.0.1 REQUIRED)
#
#   target_link_libraries(myproject libfronthaul::ecpri_u)
#
# By avoiding 'make install's we can modify the library and just rebuild, and changes are
# automatically visible in dependant projects.
#
# These are all available targets (libraries) under the libfronthaul:: namespace:
#       - ecpri_u   (userplane    protocol implementation of ecpri according to spec.)
#       - ecpri_c   (controlplane protocol implementation of ecpri interface, made with Protobuf)
#       - utils     (some various utilities for writing fronthaul-capable applications)
#      (- control_interface  (usable classes for implementing the control interface, tries to hide Protobuf details)
#      (- bbu       (not an executable, but a component for BBU that provides fronthaul connectivity)
#      (- fn        (not an executable, but a component for FN  that provides fronthaul connetivity)
#
# Thanks to Cmake, you should be able to include necessary headers (e.g. <fronthaul/eCPRI_u.hpp>)
# with no additional setup. The dependencies (include paths, for instance) propagate through the
# target_link_libraries command.

# Author: Markus Holmström (MawKKe) 2018
#
# TODO:
#	- make the libraries shared instead of static, so there would be no need to recompile the
#     client applications every time this lib changes slightly? Although, changes in interfaces
#     would cause late crashes only during runtime.
#   -

# Set minimum version of CMake.
cmake_minimum_required(VERSION 3.5) # GENERATOR_IS_MULTI_CONFIG

project(unixburrito VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)          # c++14 for now, might move to c++17 in the future
set(CMAKE_CXX_EXTENSIONS OFF)       # the default is to use -std=gnu++XX, which broke my build :( this opt makes the compiler use the "ordinary one"
set(CMAKE_CXX_STANDARD_REQUIRED ON) # prevents the standard version from "decaying" to older one if the compiler does not support the current one
                                    # Use newer compiler, you fool. Docker is here, you have no excuses.

# Common compiler definitions
#add_compile_options(-Wall)
#add_compile_options(-Wextra)
#add_compile_options(-pedantic)
#add_compile_options(-frecord-gcc-switches) # invoke "readelf -p .GCC.command.line <binary>" to see all compile time flags

# Create targets
#add_library(bar "Source/foo/Bar.cpp" "Source/foo/Bar.hpp")
#add_library(baz "Source/foo/Baz.cpp" "Source/foo/Baz.hpp")

# -----------------------------------------------------------------------
# TARGET CREATION
# -----------------------------------------------------------------------

add_library(inet src/inet.cc)
add_library(signals src/signals.cc)
#add_library(epoll   src/signals.cc)

# For simple one-way demonstration. (NOTE: not proper test programs)
add_executable(demo src/main.cc)

add_library(UnixBurrito::inet ALIAS inet)
add_library(UnixBurrito::signals ALIAS signals)

add_custom_target(link_srv ALL COMMAND ${CMAKE_COMMAND} -E create_symlink demo "server")
add_custom_target(link_cli ALL COMMAND ${CMAKE_COMMAND} -E create_symlink demo "client")
# -----------------------------------------------------------------------
# TARGET INCLUDES
# -----------------------------------------------------------------------

target_include_directories(inet
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_include_directories(signals
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# -----------------------------------------------------------------------
# TARGET LINKING
# -----------------------------------------------------------------------
target_link_libraries(demo inet signals)

# Let's change the generated file names to something descriptive and less
# prone to collisions.
#
#   For example:, "libinet.a" will now be called "libunixburrito_inet.a"
#
# Hint for future: Don't replace PREFIX, all *nix tools expect "lib"-prefix.
set_target_properties(inet    PROPERTIES OUTPUT_NAME "unixburrito_inet")
set_target_properties(signals PROPERTIES OUTPUT_NAME "unixburrito_signals")

####
# Properties of targets

# Add definitions for targets
# Values:
#   * Debug: -DFOO_BAR_DEBUG=1
#   * Release: -DFOO_BAR_DEBUG=0
#   * other: -DFOO_BAR_DEBUG=0
#target_compile_definitions(bar PUBLIC "FOO_BAR_DEBUG=$<CONFIG:Debug>")
#target_compile_definitions(baz PUBLIC "FOO_BAZ_DEBUG=$<CONFIG:Debug>")

# Generate:
#   * ${CMAKE_CURRENT_BINARY_DIR}/generated_headers/foo/BAR_EXPORT.h with BAR_EXPORT
#   * ${CMAKE_CURRENT_BINARY_DIR}/generated_headers/foo/BAZ_EXPORT.h with BAZ_EXPORT
# Renaming because:
# * We need prefix 'foo' to fit OSX/iOS frameworks layout
# * File name match name of the macro
#set(generated_headers "${CMAKE_CURRENT_BINARY_DIR}/generated_headers")
#set(bar_export "${generated_headers}/foo/BAR_EXPORT.h")
#set(baz_export "${generated_headers}/foo/BAZ_EXPORT.h")
#include(GenerateExportHeader)
#generate_export_header(bar EXPORT_FILE_NAME ${bar_export})
#generate_export_header(baz EXPORT_FILE_NAME ${baz_export})

# Global includes. Used by all targets
# Note:
#   * header location in project: Foo/Source/foo/Bar.hpp
#   * header can be included by C++ code `#include <foo/Bar.hpp>`
#   * header location in project: ${CMAKE_CURRENT_BINARY_DIR}/generated_headers/foo/BAR_EXPORT.h
#   * header can be included by: `#include <BAR_EXPORT.h>`

#target_include_directories(
#    baz PUBLIC
#    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/Source>"
#    "$<BUILD_INTERFACE:${generated_headers}>"
#)

# Link required library
#target_link_libraries(bar PUBLIC baz)

# Installation (https://github.com/forexample/package-example) {

# I guess this is needed so users of this library can use our internal targets, e.g Fronthaul::userplane
set_target_properties(${PROJET_NAME} PROPERTIES EXPORT_NAME UnixBurrito)

# Introduce variables:
# * CMAKE_INSTALL_LIBDIR
# * CMAKE_INSTALL_BINDIR
# * CMAKE_INSTALL_INCLUDEDIR
#include(GNUInstallDirs)

# Layout. This works for all platforms:
#   * <prefix>/lib*/cmake/<PROJECT-NAME>
#   * <prefix>/lib*/
#   * <prefix>/include/
#set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

#set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}")

# Configuration
#set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
#set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
#set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
#set(namespace "${PROJECT_NAME}::")

# Include module with fuction 'write_basic_package_version_file'
#include(CMakePackageConfigHelpers)

# Configure '<PROJECT-NAME>ConfigVersion.cmake'
# Use:
#   * PROJECT_VERSION
#write_basic_package_version_file(
#    "${version_config}" COMPATIBILITY SameMajorVersion
#)

# Configure '<PROJECT-NAME>Config.cmake'
# Use variables:
#   * TARGETS_EXPORT_NAME
#   * PROJECT_NAME
#configure_package_config_file(
#    "cmake/Config.cmake.in"
#    "${project_config}"
#    INSTALL_DESTINATION "${config_install_dir}"
#)

# Targets:
#   * <prefix>/lib/libbar.a
#   * <prefix>/lib/libbaz.a
#   * header location after install: <prefix>/include/foo/Bar.hpp
#   * headers can be included by C++ code `#include <foo/Bar.hpp>`
#install(
#    TARGETS inet signals
#    EXPORT "${TARGETS_EXPORT_NAME}"
#    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
#    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
#    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
#    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
#)

# Headers:
#   * Source/foo/Bar.hpp -> <prefix>/include/foo/Bar.hpp
#   * Source/foo/Baz.hpp -> <prefix>/include/foo/Baz.hpp
#install(
#    DIRECTORY "include"
#    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
#    FILES_MATCHING PATTERN "*.hpp"
#)

# Export headers:
#   * ${CMAKE_CURRENT_BINARY_DIR}/.../BAR_EXPORT.h -> <prefix>/include/foo/BAR_EXPORT.h
#   * ${CMAKE_CURRENT_BINARY_DIR}/.../BAZ_EXPORT.h -> <prefix>/include/foo/BAZ_EXPORT.h
#install(
#    FILES "${bar_export}" "${baz_export}"
#    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/foo"
#)

# Config
#   * <prefix>/lib/cmake/Foo/FooConfig.cmake
#   * <prefix>/lib/cmake/Foo/FooConfigVersion.cmake
#install(
#    FILES "${project_config}" "${version_config}"
#    DESTINATION "${config_install_dir}"
#)

# Config
#   * <prefix>/lib/cmake/Foo/FooTargets.cmake
#install(
#    EXPORT "${TARGETS_EXPORT_NAME}"
#    NAMESPACE "${namespace}"
#    DESTINATION "${config_install_dir}"
#)

#export(EXPORT "${TARGETS_EXPORT_NAME}" FILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGETS_EXPORT_NAME}.cmake NAMESPACE ${namespace})

#export(PACKAGE ${PROJECT_NAME})



# Tests? ---------------------------------------------------

# TODO
#include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.gtest.txt)
#ENABLE_TESTING()

#add_executable(test_inet    tests/main.cc tests/test_inet.cc)
#add_executable(test_signals tests/main.cc tests/test_signals.cc)
#add_executable(test_epoll  tests/main.cc tests/test_epoll.cc)

#target_link_libraries(test_inet    unixburrito GTest::GTest)
#target_link_libraries(test_signals unixburrito GTest::GTest)
#target_link_libraries(test_epoll   unixburrito GTest::GTest)

#add_test(Inet    test_inet)
#add_test(Signals test_signals)
#add_test(Epoll   test_epoll)

# }
