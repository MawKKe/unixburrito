cmake_minimum_required(VERSION 2.8.2)

project(googletest-download NONE)

include(ExternalProject)
ExternalProject_Add(googletest
  #  GIT_REPOSITORY    https://github.com/google/googletest.git
  #  GIT_TAG           master
  URL https://github.com/google/googletest/archive/release-1.8.1.tar.gz
  URL_HASH SHA256=9bf1fe5182a604b4135edc1a425ae356c9ad15e9b23f9f12a02e80184c3a249c
  SOURCE_DIR        "${CMAKE_CURRENT_BINARY_DIR}/ext/googletest-src"
  BINARY_DIR        "${CMAKE_CURRENT_BINARY_DIR}/ext/googletest-build"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)
