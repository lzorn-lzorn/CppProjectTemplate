#pragma once
#include <type_traits>
namespace Math{

template <typename Tp>
concept Arithmetic = requires {
    requires std::is_arithmetic_v<Tp>;
    requires !std::is_same_v <std::remove_cvref_t<Tp>, bool>;
    requires !std::is_same_v <std::remove_cvref_t<Tp>, char>;
};


template <Arithmetic Tp, size_t Row, size_t Col>
class Matrix;
template <Arithmetic Tp, size_t N>
class Matrix<Tp, N, N>;
}