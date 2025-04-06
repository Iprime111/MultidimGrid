cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project(multidim-grid)

# Create lib target
add_library(multidim-grid INTERFACE)
target_include_directories(multidim-grid INTERFACE include)
target_compile_features(multidim-grid INTERFACE cxx_std_20)

add_library(grid::grid ALIAS multidim-grid)

if(PROJECT_IS_TOP_LEVEL)
    enable_testing()

    include(cmake/CPM.cmake)

    CPMAddPackage(
      NAME googletest
      GITHUB_REPOSITORY google/googletest
      GIT_TAG release-1.12.1
      VERSION 1.12.1
    )

    # Create tests target
    add_executable(multidim-grid-tests)
    
    # TODO test debug and release compile options
    
    target_compile_options(multidim-grid-tests PRIVATE -Wall)
    
    # Link libs
    
    target_link_libraries(multidim-grid-tests PRIVATE multidim-grid GTest::gtest_main)
    
    add_subdirectory(tests)
    
    # Discover tests
    
    include(GoogleTest)
    gtest_discover_tests(multidim-grid-tests)
endif()
