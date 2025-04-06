cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project(multidim-grid)

enable_testing()

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME googletest
  GITHUB_REPOSITORY google/googletest
  GIT_TAG release-1.12.1
  VERSION 1.12.1
)

# Create lib target

add_library(multidim-grid-lib INTERFACE)
target_include_directories(multidim-grid-lib INTERFACE include)
target_compile_features(multidim-grid-lib INTERFACE cxx_std_20)

# Create tests target

add_executable(multidim-grid-tests)

# TODO test debug and release compile options

target_compile_options(multidim-grid-tests PRIVATE -Wall)

# Link libs

target_link_libraries(multidim-grid-tests PRIVATE multidim-grid-lib GTest::gtest_main)

add_subdirectory(tests)

# Discover tests

include(GoogleTest)
gtest_discover_tests(multidim-grid-tests)
