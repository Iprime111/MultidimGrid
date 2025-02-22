#include <exception>
#include <gtest/gtest.h>
#include <string>
#include <utility>

#include "grid.hpp"

TEST(GridTests, BracedInitializationTest) {
    // Checking usual braced-list-initializatioon
    grid::Grid<int, 1> grid1{0, 1, 2, 3};
    EXPECT_EQ(grid1.size(0), 4);

    grid::Grid<int, 2> grid2{{0, 1}, {2, 3}, {4, 5}, {6, 7}};
    EXPECT_EQ(grid2.size(0), 4);
    EXPECT_EQ(grid2.size(1), 2);

    grid::Grid<int, 3> grid3{{{0, 1}, {2, 3}, {4, 5}, {6, 7}},
                             {{0, 1}, {2, 3}, {4, 5}, {6, 7}},
                             {{0, 1}, {2, 3}, {4, 5}, {6, 7}},
                             {{0, 1}, {2, 3}, {4, 5}, {6, 7}}};

    EXPECT_EQ(grid3.size(0), 4);
    EXPECT_EQ(grid3.size(1), 4);
    EXPECT_EQ(grid3.size(2), 2);

    // Expecting to get an error because of invalid shape of the grid
    try {
        grid::Grid<int, 2> gridError{{0, 1}, {2}, {4, 5}, {6, 7}};
        ASSERT_TRUE(false);
    } catch(const std::exception&) {}
}

TEST(GridTests, InitializationBySizeTest) {
    // Checking direct initialization with grid dimensions
    grid::Grid<int, 5> grid(1, 2, 3, 4, 5);

    EXPECT_EQ(grid.size(0), 1);
    EXPECT_EQ(grid.size(1), 2);
    EXPECT_EQ(grid.size(2), 3);
    EXPECT_EQ(grid.size(3), 4);
    EXPECT_EQ(grid.size(4), 5);

    // Expecting to get an error because of the zero size of the grid (in 4rth dimension)
    try {
        grid::Grid<int, 5> grid(1, 2, 3, 0, 5);
        ASSERT_TRUE(false);
    } catch(const std::exception&) {}
}

TEST(GridTests, CopyAndMoveConstructionTest) {
    grid::Grid<int, 2> grid1{{1, 0},
                             {0, 1}};

    // Test copy constructor
    auto grid2{grid1};
    EXPECT_EQ(grid1, grid2);

    // Test copy assignment
    auto grid3 = grid1;
    EXPECT_EQ(grid1, grid3);

    // Test move constructor
    auto grid4{std::move(grid2)};
    EXPECT_EQ(grid1, grid4);

    // Test move assignment
    auto grid5 = std::move(grid3);
    EXPECT_EQ(grid1, grid5);

    grid::Grid<std::string, 1> strGrid{"aaa", "bbb", "ccc"};

    auto strGridCpy = strGrid;
    EXPECT_EQ(strGrid, strGridCpy);

    auto strGridMoved = std::move(strGridCpy);
    EXPECT_EQ(strGrid, strGridMoved);
}

TEST(GridTests, EqualityOperatorsTest) {

    grid::Grid<int, 3> grid{{{1, 2, 3}, {3, 4, 5}},
                            {{1, 2, 3}, {5, 6, 7}}};

    auto grid2{grid};
    EXPECT_EQ(grid, grid2);

    grid2[0][0][0] = 0;
    EXPECT_NE(grid, grid2);
}

TEST(GridTests, SizeFunctionFailTest) {
    grid::Grid<int, 5> grid(1, 2, 3, 4, 5);

    try {
        grid.size(10);
        ASSERT_TRUE(false);
    } catch(const std::exception&) {}
}

TEST(GridTests, SubgridIterationTest) {
    grid::Grid<int, 3> grid{{{1, 1}, {1, 1}, {1, 1}, {1, 1}},
                            {{1, 1}, {1, 1}, {1, 1}, {1, 1}},
                            {{1, 1}, {1, 1}, {1, 1}, {1, 1}},
                            {{1, 1}, {1, 1}, {1, 1}, {1, 1}}};

    for(auto& subgrid2d : grid) {
        for(auto& subgrid1d : subgrid2d) {
            for (auto& val : subgrid1d) {
                ASSERT_EQ(val, 1);

                val = 5;
            }
        }
    }
}

TEST(GridTests, ReduceTest) {
    grid::Grid<int, 3> grid{{{1, 2}, {3, 4}, {5, 6}, {7, 8}},
                            {{1, 2}, {3, 4}, {5, 6}, {7, 8}},
                            {{1, 2}, {3, 4}, {5, 6}, {7, 8}},
                            {{1, 2}, {3, 4}, {5, 6}, {7, 8}}};

    // Test different traversal directions and argument passing
    int sum{0};
    grid.reduce([&sum](auto &value, auto&) {
        sum += value;
    }, grid::NoDimension, grid::NoDimension, grid::NoDimension);
    EXPECT_EQ(sum, 36 * 4);

    sum = 0;
    grid.reduce([&sum](int &value, auto&) {
        sum += value;
    }, grid::NoDimension, 0, 0);
    EXPECT_EQ(sum, 4);

    sum = 0;
    grid.reduce([&sum](const int &value, const auto&) {
        sum += value;
    }, 0, grid::NoDimension,  0);
    EXPECT_EQ(sum, 1 + 3 + 5 + 7);

    sum = 0;
    grid.reduce([&sum](int value, auto&) { 
        sum += value;
    }, 0, 0, grid::NoDimension);
    EXPECT_EQ(sum, 3);

    sum = 0;
    grid.reduce([&sum](const int value, auto&) { 
        sum += value;
    }, 2, grid::NoDimension, grid::NoDimension);
    EXPECT_EQ(sum, 36);

    sum = 0;
    grid.reduce([](int &value, auto&){
        value = 0;
    }, grid::NoDimension, grid::NoDimension, grid::NoDimension);
    grid.reduce([&sum](int value, auto&) {
        sum += value;
    }, grid::NoDimension, grid::NoDimension, grid::NoDimension);
    EXPECT_EQ(sum, 0);

    grid::Grid<int, 2> coordGrid{{0, 1, 2},
                                 {1, 2, 3},
                                 {2, 3, 4}};

    coordGrid.reduce([](auto& val, auto& coords) {
        EXPECT_EQ(val, coords[0] + coords[1]);
    }, grid::NoDimension, grid::NoDimension);
}

TEST(GridTests, TraverseTest) {
    grid::Grid<int, 1> grid{1, 2, 3, 4, 5};

    int sum{0};
    grid.traverse([&sum](const int& val, auto&) {
        sum += val; 
        if (val == 4) {
            return true;
        }

        return false;
    }, grid::NoDimension);

    EXPECT_EQ(sum, 10);
}

TEST(GridTests, AccessOperatorTest) {
    grid::Grid<int, 3> grid{{{1, 2}, {3, 4}, {5, 6}, {7, 8}},
                            {{1, 2}, {3, 4}, {5, 6}, {7, 8}},
                            {{1, 2}, {3, 4}, {5, 6}, {7, 8}},
                            {{1, 2}, {3, 4}, {5, 6}, {7, 8}}};

    // Testing reading
    EXPECT_EQ(grid[3][0][0], 1);
    EXPECT_EQ(grid[3][0][1], 2);
    EXPECT_EQ(grid[2][1][0], 3);
    EXPECT_EQ(grid[2][1][1], 4);
    EXPECT_EQ(grid[1][2][0], 5);
    EXPECT_EQ(grid[1][2][1], 6);
    EXPECT_EQ(grid[0][3][0], 7);
    EXPECT_EQ(grid[0][3][1], 8);

    // Testing modification
    EXPECT_EQ(++grid[1][3][0], 8);

    EXPECT_EQ(std::exchange(grid[1][0][0], 5), 1);
    EXPECT_EQ(grid[1][0][0], 5);

    

    // Testing movement
    grid::Grid<std::string, 2> stringGrid{{"aaa", "bbb"},
                                          {"ccc", "ddd"}};
    
    std::string newVal = "eee";
    std::swap(stringGrid[1][1], newVal);

    EXPECT_EQ(stringGrid[1][1], "eee");
    EXPECT_EQ(newVal, "ddd");
}
