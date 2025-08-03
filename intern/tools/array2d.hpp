#pragma once
#include <cstdint>
#include <memory>
#include <vector>
namespace Tools {

template <class Array>
struct ConstIterator {};

template <class Array>
struct Iterator : public ConstIterator<Array> {

};

template <class Array>
struct PointerTraits : public std::pointer_traits<Array> {};
template <class Array>
struct PointerTraits <ConstIterator<Array>>{};
template <class Array>
struct PointerTraits <Iterator<Array>>{};

template <class ValueTp, class SizeTp, class DiffTp, class Pointer, class ConstPoint>
struct IteratorTypes {
    using value_type      = ValueTp;
    using size_type       = SizeTp;
    using difference_type = DiffTp;
    using pointer         = Pointer;
    using const_pointer   = ConstPoint;
};

template <typename Tp, class Allocator=std::allocator<Tp>>
class Array2D : public std::vector<Tp, Allocator>{
public:
    using base            = std::vector<Tp, Allocator>;
    using value_type      = base::value_type;
    using allocator_type  = base::allocator_type;
    using pointer         = base::pointer;
    using const_pointer   = base::const_pointer;
    using reference       = base::reference;
    using const_reference = base::const_reference;
    using size_type       = base::size_type;
    using difference_type = base::difference_type;
    
public:
    /* 使用原本vector的方法 */
    using base::empty;
    using base::size;
    using base::clear;

    /* 禁用原本vector的方法 */

public:
    Array2D(const uint32_t row, const uint32_t col) 
        : row(row), col(col) {}
    Array2D(const Array2D&) = default;
    Array2D(Array2D&&) = default;
    Array2D& operator=(const Array2D&) = default;
    Array2D& operator=(Array2D&&) = default;
    ~Array2D() = default;
    bool operator==(const Array2D& other) const;

private:


public:

private:
    uint32_t row, col;
};

template <class Tp, class Alloc>
bool operator==(const Array2D<Tp, Alloc>& Left, const Array2D<Tp, Alloc>& Right){

    return true;
}
template <class Tp, class Alloc>
bool operator!=(const Array2D<Tp, Alloc>& Left, const Array2D<Tp, Alloc>& Right){

    return true;
}
template <class Tp, class Alloc>
bool operator<(const Array2D<Tp, Alloc>& Left, const Array2D<Tp, Alloc>& Right){

    return true;
}
template <class Tp, class Alloc>
bool operator>(const Array2D<Tp, Alloc>& Left, const Array2D<Tp, Alloc>& Right){

    return true;
}
template <class Tp, class Alloc>
bool operator<=(const Array2D<Tp, Alloc>& Left, const Array2D<Tp, Alloc>& Right){

    return true;
}
template <class Tp, class Alloc>
bool operator>=(const Array2D<Tp, Alloc>& Left, const Array2D<Tp, Alloc>& Right){

    return true;
}
template <class Tp, class Alloc>
void swap(Array2D<Tp, Alloc>& Left, Array2D<Tp, Alloc>& Right){}

template <class Tp, class Alloc, class ValTp>
constexpr Array2D<Tp, Alloc>::size_type erase(Array2D<Tp, Alloc>& array2d, const ValTp& val){}

template <class Tp, class Alloc, class Predicate>
constexpr Array2D<Tp, Alloc>::size_type erase_if(Array2D<Tp, Alloc>& array2d, Predicate pred){}


}