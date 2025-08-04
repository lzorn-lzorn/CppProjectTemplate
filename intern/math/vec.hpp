#include <array>
#include <cmath>
#include <execution>
#include <stdexcept>
#include "traits.hpp"
#include "../tools/static_for.hpp"
namespace Math {

template <Arithmetic Tp, size_t Dimension>
class Vector{
public:
    Vector& Normalize(){
        const double len = Length();
        if (len <= std::numeric_limits<double>::epsilon()) [[unlikely]] {
            #ifndef NDEBUG
                throw std::runtime_error("Vector Normalize Divide 0");
            #endif
            return *this; 
        }

        /* 并行计算每个分量的归一化值 */ 
        std::transform(
            std::execution::par,
            vec.begin(), vec.end(),
            vec.begin(),
            [len](double x) { return x / len; }
        );

        return *this;
    }
    double Length() const noexcept {
        return std::sqrt(Square());
    }
    double Square() const noexcept{
        return std::transform_reduce(
            std::execution::par,        /* 并行执行 */ 
            vec.begin(), vec.end(),     /* 输入范围 */ 
            0.0,                        /* 初始值 */ 
            std::plus<>(),              /* 归约操作（加法） */ 
            [](double x) {              // 
                return x * x;           /* 变换操作（平方） */ 
            }
        );
    }
private:
    std::array<Tp, Dimension> vec;
};

template <Arithmetic Tp>
class Vector<Tp, 1>{};

}