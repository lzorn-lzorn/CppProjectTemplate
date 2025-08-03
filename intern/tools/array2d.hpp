#pragma once
#include <cstdint>
#include <memory>
#include <vector>
namespace Tools {

template <typename Tp, class Allocator=std::allocator<Tp>>
class Array2D : public std::vector<Tp, Allocator>{
public:
    using value_type = Tp;
    using base = std::vector<Tp, Allocator>;
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
    class Iterator {
        typename base::iterator base_iter;
        uint32_t col_;
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = Tp;
        using difference_type = std::ptrdiff_t;
        using pointer = Tp*;
        using reference = Tp&;

        Iterator(typename base::iterator it, uint32_t col) 
            : base_iter(it), col_(col) {}

        reference operator*() { return *base_iter; }
        Iterator& operator++() { ++base_iter; return *this; }
        bool operator!=(const Iterator& other) const {
            return base_iter != other.base_iter;
        }
        // 其他迭代器操作...
        value_type& operator[](const uint32_t col);
        value_type& operator[](const uint32_t col) const;
    };

public:
    Iterator begin();
    Iterator end();
    Iterator operator[](const uint32_t row);
    Iterator begin() const;
    Iterator end() const;
    Iterator operator[](const uint32_t row) const;
private:
    uint32_t row, col;
};
}