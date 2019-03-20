# Usage:
#   mkdir build && cd build && cmake ..
#
# Author: Markus Holmström (MawKKe) 2018
#
# Set minimum version of CMake.
cmake_minimum_required(VERSION 3.5) # GENERATOR_IS_MULTI_CONFIG

project(unixburrito VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)          # c++14 for now, might move to c++17 in the future
set(CMAKE_CXX_EXTENSIONS OFF)       # the default is to use -std=gnu++XX, which broke my build :( this opt makes the compiler use the "ordinary one"
set(CMAKE_CXX_STANDARD_REQUIRED ON) # prevents the standard version from "decaying" to older one if the compiler does not support the current one
                                    # Use newer compiler, you fool. Docker is here, you have no excuses.

option(UNIXBURRITO_DEVELOP "Enable development of the unixburrito library itself" OFF)

if(UNIXBURRITO_DEVELOP)
    # Downloads gtest into ext/ and provides targets gtest and gtest_main
    include(${CMAKE_SOURCE_DIR}/cmake/gtest.cmake)

    # Common useful compiler definitions
    # These should only be enabled when we are doing development on the library itself.
    # Otherwise we should use only the compile options that the parent project uses.
    add_compile_options(-Wall)
    add_compile_options(-Wextra)
    add_compile_options(-pedantic)
    add_compile_options(-frecord-gcc-switches) # invoke "readelf -p .GCC.command.line <binary>" to see all compile time flags
endif()


# -----------------------------------------------------------------------
# TARGET CREATION
# -----------------------------------------------------------------------

add_library(inet src/inet.cc)
add_library(signals src/signals.cc)
add_library(cpuset  src/cpuset.cc)
#add_library(epoll   src/signals.cc)

# For simple one-way demonstration. (NOTE: not proper test programs)
add_executable(demo src/main.cc)

add_library(UnixBurrito::inet ALIAS inet)
add_library(UnixBurrito::signals ALIAS signals)
add_library(UnixBurrito::cpuset ALIAS cpuset)

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

target_include_directories(cpuset
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# -----------------------------------------------------------------------
# TARGET LINKING
# -----------------------------------------------------------------------
target_link_libraries(cpuset pthread)
target_link_libraries(demo inet signals cpuset)

# -----------------------------------------------------------------------
# ADDITIONAL PROPERTIES
# -----------------------------------------------------------------------

# Let's change the generated file names to something descriptive and less
# prone to collisions.
#
#   For example:, "libinet.a" will now be called "libunixburrito_inet.a"
#
# Hint for future: Don't replace PREFIX, all *nix tools expect "lib"-prefix.
set_target_properties(inet    PROPERTIES OUTPUT_NAME "unixburrito_inet")
set_target_properties(signals PROPERTIES OUTPUT_NAME "unixburrito_signals")
set_target_properties(cpuset  PROPERTIES OUTPUT_NAME "unixburrito_cpuset")


# Testing, only if we are developing

if(UNIXBURRITO_DEVELOP)
    add_subdirectory(tests)
endif()

