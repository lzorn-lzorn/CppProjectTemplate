#include <array>
#include "traits.hpp"
namespace Math {

template <Arithmetic Tp, size_t Dimension>
class Vector{
public:

private:
    std::array<Tp, Dimension> vec;
};

template <Arithmetic Tp>
class Vector<Tp, 1>{};

}