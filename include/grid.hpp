#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "traits.hpp"

namespace grid {

struct NoDimensionType {
    enum ConstructorToken_ { Token_ };

    NoDimensionType() = delete;
    constexpr inline explicit NoDimensionType(ConstructorToken_) {}
};

constexpr inline NoDimensionType NoDimension{NoDimensionType::ConstructorToken_::Token_};

template<typename T>
concept Size = std::convertible_to<T, std::size_t>;

template<typename T>
concept Dimension = Size<T> || std::same_as<T, NoDimensionType>;

template<typename T, std::size_t Dimensionality>
class Grid {
    template<typename U, std::size_t OtherDimensionality>
    friend class Grid;

    static_assert(Dimensionality > 0, "Zero-dimensional grids are not allowed");

  public:
    using ValueType = std::conditional_t<(Dimensionality > 1), Grid<T, Dimensionality - 1>, T>;

  private:
    std::vector<ValueType> subgrids_;

  public:
    using InnerContainerType = decltype(subgrids_);

    using ConstIterator = InnerContainerType::const_iterator;
    using Iterator = InnerContainerType::iterator;
    using ConstReference = InnerContainerType::const_reference;
    using Reference = InnerContainerType::reference;

    // For compatibility with stl functions
    using iterator = Iterator;
    using const_iterator = ConstIterator;
    using reference = Reference;
    using const_reference = ConstReference;

    explicit Grid() = delete;

    Grid(const Grid<T, Dimensionality>&) = default;
    Grid(Grid<T, Dimensionality>&&) = default;

    Grid<T, Dimensionality>& operator=(const Grid<T, Dimensionality>&) = default;
    Grid<T, Dimensionality>& operator=(Grid<T, Dimensionality>&&) = default;

    bool operator==(const Grid<T, Dimensionality>& other) const = default;
    bool operator!=(const Grid<T, Dimensionality>& other) const = default;
    
    auto begin() const noexcept { return subgrids_.begin(); }
    auto begin() noexcept { return subgrids_.begin(); }
    
    auto end() const noexcept { return subgrids_.end(); }
    auto end() noexcept { return subgrids_.end(); }

    Reference operator[](std::size_t index) { return subgrids_[index]; }
    ConstReference operator[](std::size_t index) const { return subgrids_[index]; }

    Reference at(std::size_t index) { return subgrids_.at(index); }
    ConstReference at(std::size_t index) const { return subgrids_.at(index); }

    template<Size... Dimensions>
    explicit Grid(Dimensions... dimensions) {
        static_assert(sizeof...(dimensions) == Dimensionality, "Invalid count of grid dimensions");

        const std::size_t currentDimension = utils::packFirstElementV(dimensions...);

        if (currentDimension == 0) {
            throw std::runtime_error("Grid size must be greater than zero in every dimension");
        }

        subgrids_.reserve(currentDimension);

        for (std::size_t subgridIdx = 0; subgridIdx < currentDimension; subgridIdx++) {
            [this](auto, auto... args) {
                // Here we're using the fact that emplace_back uses std::allocator_traits<Alloc>::construct 
                // which uses std::construct_at (since C++20) which uses placement new operator with direct initialization
                // So, in our case the same constructor will be called for the Grid<T, Dimensionality - 1>
                subgrids_.emplace_back(args...); // No need for std::forward because dimensions are all size_t&
            }(dimensions...);
        }
    }

    Grid(std::initializer_list<ValueType> subgrids) : subgrids_(std::move(subgrids)) {
        if (subgrids_.size() == 0) {
            throw std::out_of_range("Grid size must be greater than zero in every dimension");
        }

        if constexpr (Dimensionality > 1) {
            const std::size_t firstSize = subgrids_[0].size(0);

            for (const auto &subgrid : subgrids_) {
                if (subgrid.size(0) != firstSize) {
                    throw std::runtime_error("Grid must have a shape of an N-dimensional rectangular parallelepiped");
                }
            }
        }
    } 

    template<Dimension... Dimensions>
    bool traverse(const std::function<bool(T&, const std::array<std::size_t, Dimensionality>&)>& predicate, 
                  Dimensions... dimensions) {

        static_assert(sizeof...(dimensions) == Dimensionality, "Invalid count of grid dimensions");

        std::array<std::size_t, Dimensionality> coords{};
        return traverseImpl(coords, predicate, dimensions...);
    }

    template<Dimension... Dimensions>
    void reduce(const std::function<void(T&, const std::array<std::size_t, Dimensionality>&)>& predicate, 
                Dimensions... dimensions) {
        
        static_assert(sizeof...(dimensions) == Dimensionality, "Invalid count of grid dimensions");

        traverse([&predicate](T& value, auto& coords) { 
            predicate(value, coords);
            return false;
        }, dimensions...);
    }

    std::size_t size(std::size_t dimension) const {
        if (dimension == 0) {
            return subgrids_.size();    
        }

        if (subgrids_.size() == 0) {
            return 0;
        }

        if constexpr (Dimensionality > 1) {
            return subgrids_[0].size(dimension - 1);
        }
        
        throw std::runtime_error("Dimension value is too big");
    }

    std::size_t size() const { return subgrids_.size(); }

  private: 
    template<Dimension... Dimensions, std::size_t CoordinatesCount>
    bool traverseImpl(std::array<std::size_t, CoordinatesCount>& coords, 
                      const std::function<bool(T&, const std::array<std::size_t, CoordinatesCount>&)>& predicate, 
                      Dimensions... dimensions) {

        constexpr bool isNoDimension = utils::CompareFirstElemType<NoDimensionType>(dimensions...);

        if constexpr (isNoDimension) {
            for (auto &subgrid : subgrids_) {
                // Here we're just exploiting pointers capabilities. This code is kinda dependent on the std::distance implementation.
                // This happens because std::distance is expecting an argument to meet the requirements of (at least) LegacyInputIterator
                // (or LegacyRandomAccessIterator to have O(1) complexity). Pointers actually don't satisfy this requirements,
                // as they don't have value_type, reference and difference_type types defined. Despite this, std::distance will
                // (nearly) always work without those types being defined :)
                coords[CoordinatesCount - Dimensionality] = std::distance(&subgrids_.front(), &subgrid);

                if constexpr (Dimensionality > 1) {
                    if ([&coords, &predicate, &subgrid](auto firstDim, auto... other) {
                        return subgrid.traverseImpl(coords, predicate, other...);
                    }(dimensions...)) {
                        return true;
                    }
                } else {
                    if(predicate(subgrid, coords)) {
                        return true;
                    }
                }
            }

            return false;
        } else {
            return [this, &coords, &predicate](auto firstDim, auto... other) {
                coords[CoordinatesCount - Dimensionality] = firstDim;

                if constexpr (Dimensionality > 1) {
                    return subgrids_[firstDim].traverseImpl(coords, predicate, other...);
                } else {
                    return predicate(subgrids_[firstDim], coords);
                }
            }(dimensions...);
        }
    }

};
} // namespace grid
