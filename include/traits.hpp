#pragma once

#include <type_traits>
#include <utility>

namespace grid {
namespace utils {

template<typename FirstElem, typename... OtherElems>
constexpr FirstElem&& packFirstElementV(FirstElem&& firstElem, OtherElems&&...) {
    return std::forward<FirstElem>(firstElem);
}

template<typename FirstElem, typename... OtherElems>
struct packFirstElement {
    using Type = FirstElem;

	template<typename T, typename... U>
    explicit constexpr packFirstElement(T&&, U&&...) {}
};

template<typename T, typename... U>
explicit packFirstElement(T&&, U&&...) -> packFirstElement<T, U...>;

template<typename DesiredType, typename... Args>
constexpr bool CompareFirstElemType(Args&&... args) {
    using ArgType = std::remove_cvref_t<typename decltype(packFirstElement(std::forward<Args>(args)...))::Type>;
    using Desired = std::remove_cvref_t<DesiredType>;

    return std::is_same_v<Desired, ArgType>;
}

template<typename DesiredType, typename FirstElem, typename... OtherElems>
constexpr std::size_t countEqualType(FirstElem&& firstElem, OtherElems&&... other) {
    std::size_t nextResult = 0;
    
    if constexpr (sizeof...(other) > 0) {
        nextResult = countEqualType<DesiredType>(std::forward<OtherElems>(other)...);
    }

    using ElemType = std::remove_cvref_t<decltype(firstElem)>;
    using DesiredTypeNoCv = std::remove_cvref_t<DesiredType>;

    if constexpr (std::is_same_v<DesiredTypeNoCv, ElemType>) {
        return 1 + nextResult;
    } else {
        return nextResult;
    }
}

} // namespace utils
} // namespace grid
