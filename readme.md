# 🧮 Multidimensional Grid Library (C++20)

A lightweight and efficient C++20 library for handling multidimensional grids with flexible element traversal and reduction.

## 🚀 Features

- **📦 N-Dimensional Grids**: `Grid<T, N>` template class
- **🔍 Element Access**: 
  - Safe `at()` with bounds checking
  - Unchecked `operator[]` chaining
- **🖨️ Printing**:
  - Direct output for 1D/2D grids (`<<` operator)
  - 2D slice printing for higher dimensions (`printSubgrid()`)
- **🔄 Iterators**: Range-based access to (N-1)D subgrids
- **⚖️ Comparisons**: Deep equality checks (`==`/`!=`)
- **🎯 Traversal/Reduction**: Process elements by coordinate patterns
- **📐 CMake Integration**: Easy embedding into projects


## 📥 Installation & Building

### Prerequisites
- C++20 compiler
- CMake 3.8+

### As Subproject (cmake)
```cmake
add_subdirectory(path/to/grid_library)
target_link_libraries(your_target PRIVATE grid::grid)
```

### Standalone Build
```bash
git clone https://github.com/Iprime111/MultidimGrid
cd MultidimGrid
mkdir build && cd build
cmake .. -GNinja && ninja
ctest
```

| Project Context      | Built Targets          |
|----------------------|------------------------|
| Top-level project    | Library + test suite   |
| Subdirectory         | Library only           |

## 🛠️ Basic Usage

### Create & Access
```cpp
grid::Grid<int, 3> cube(2, 3, 4);  // 2x3x4 cube
cube.at(1).at(2).at(0) = 42;       // Bounds-checked
cube[1][2][3] = 7;                 // Unchecked
```

### Iterate Subgrids
```cpp
// Process 2D slices of 3D grid
for (auto& slice : cube) { // Slice is Grid<int, 2>
    std::cout << slice << "\n";
}
```

### Compare Grids
```cpp
Grid<int, 2> a{{1, 2}, {3, 4}};
Grid<int, 2> b{{1, 2}, {3, 4}};
assert(a == b);
```

## 🖨️ Printing

### Direct Output (N <= 2)
```cpp
Grid<float, 2> matrix{{1.1, 2.2}, {3.3, 4.4}};
std::cout << matrix;
```
**Output:**
```
1.1 2.2
3.3 4.4
```

### 2D Slices
```cpp
Grid<int, 4> hypergrid(2, 2, 2, 2);
// Fix first two dimensions
grid::printSubgrid(std::cout, hypergrid, 
                   0, 1, grid::NoDimension, grid::NoDimension);
```
**Output:**
```
0 0
0 0
```

## 🎯 Traversal & Reduction

### Find first negative
```cpp
grid.traverse(
    [](auto& val, auto& coords) {
        if (val < 0) {
            std::cout << "Negative at " << coords[0] << ", " << coords[1];
            return true;
        }
        return false;
    },
    0, // Fix first dimension
    grid::NoDimension
);
```

### Sum third column
```cpp
int total = 0;
grid.reduce(
    [&total](auto& val, [[maybe_unused]] auto& coords) {
        total += val;
    },
    grid::NoDimension, 
    2 // Fix third column
);
```

## API Reference

### **Grid Class (`Grid<T, N>`)**  
A template class representing an N-dimensional grid of elements of type `T`.

```cpp
template<typename T, std::size_t N>
class Grid;
```

#### **1. Constructors**

- **Dimension Constructor**  
  ```cpp
  template<Size... Dimensions>
  explicit Grid(Dimensions... dimensions);
  ```  
  Constructs a grid with specified sizes for all `N` dimensions.  

- **Initializer List Constructors**  
  ```cpp
  // For N > 1
  Grid(std::initializer_list<Grid<T, N-1>> subgrids);

  // For N = 1
  Grid(std::initializer_list<T> subgrids);
  ```  
  Constructs from a nested initializer list matching the grid's dimensionality.  

- **Copy/Move Constructors**  
  ```cpp
  Grid(const Grid&);            // Copy
  Grid(Grid&&) noexcept;        // Move
  ```  
  Standard copy and move semantics.  

---

#### **2. Element Access**

- **`at()`**
  ```cpp
  // For N > 1
  Grid<T, N-1>& at(size_t index);
  const Grid<T, N-1>& at(size_t index) const;

  // For N = 1
  T& at(size_t index);
  const T& at(size_t index) const;
  ```  
  Bounds-checked access to subgrids (or elements for `N = 1`). Throws `std::out_of_range` on invalid indices.  

- **`operator[]`**
  ```cpp
  // For N > 1
  Grid<T, N-1>& operator[](size_t index);
  const Grid<T, N-1>& operator[](size_t index) const;

  // For N = 1
  T& operator[](size_t index);
  const T& operator[](size_t index) const;
  ```  
  Unchecked access returning subgrids (or elements for `N = 1`).  

---

#### **3. Iterators**

- **`begin()`/`end()`**
  ```cpp
  iterator begin() noexcept;
  const_iterator begin() const noexcept;
  iterator end() noexcept;
  const_iterator end() const noexcept;
  ```  
  Return iterators to traverse subgrids of dimensionality `N-1`.  

---

#### **4. Core Methods**

- **Equality Operators**
  ```cpp
  bool operator==(const Grid<T, N>& other) const;
  bool operator!=(const Grid<T, N>& other) const;
  ```  
  Perform deep equality checks between grids.  

- **`traverse`**  
  ```cpp
  template <typename... Dimensions>
  void traverse(const auto& pred, Dimensions... dims);
  
  template <typename... Dimensions>
  void traverse(const auto& pred, Dimensions... dims) const;
  ```  
  Applies `pred` to elements matching `dims` until `pred` returns `true`.  
  - **Parameters**:  
    - `pred` (non-const version): `bool(T&, const std::array<size_t, N>&)`
    - `pred` (const version): `bool(const T&, const std::array<size_t, N>&)`
    - `dims`: `N` constraints (indices or `grid::NoDimension`).  

- **`reduce`**  
  ```cpp
  template <typename... Dimensions>
  void reduce(const auto& func, Dimensions... dims);

  template <typename... Dimensions>
  void reduce(const auto& func, Dimensions... dims);
  ```  
  Applies `func` to all elements matching `dims`.  
  - **Parameters**:  
    - `func` (non-const version): `void(T&, const std::array<size_t, N>&)`  
    - `func` (const version): `void(const T&, const std::array<size_t, N>&)`  

---

### **Grid Namespace**

#### **1. Printing Utilities**

- **`printSubgrid`**  
  ```cpp
  namespace grid {
    template <typename T, size_t N, typename... Dimensions>
    void printSubgrid(std::ostream& os, const Grid<T, N>& grid, Dimensions... dims);
  }
  ```  
  Prints a 1D or 2D slice of a grid with.  
  - **Requirements**:  
    - `N-2` or `N-1` dimensions fixed via indices in `dims`.  
    - Remaining 2 dimensions specified as `grid::NoDimension`.  

- **`operator<<` overloads**  
  ```cpp
  namespace grid {
    // For 1D grids
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Grid<T, 1>& grid);

    // For 2D grids
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Grid<T, 2>& grid);
  }
  ```  
  Outputs elements in row `e1 e2 e3 ...` (1D) or space-separated rows (2D).  

---

#### **2. Dimension Specifications**

- **Wildcard**  
  ```cpp
  namespace grid {
    constexpr inline NoDimensionType NoDimension = /* implementation-defined */;
  }
  ```  
  Used to ignore a dimension during traversal, reduction, or printing.  

---

### **Type Requirements**

1. **Element Type (`T`)**  
   - Copy-constructible and copy-assignable.  
   - Default-constructible (required for dimension-based construction).  
   - Supports `operator==` for equality checks.  
   - Should define `operator<<` for `std::ostream` if used with printing utilities.  

## 📜 License
[MIT License](LICENSE)

## TODO List

- Rewrite at() method to work with variadic parameters
- Try to find faster replacement for the std::vector
- Allow changing grid's size
- Implement more utility methods (e.g. fill())


