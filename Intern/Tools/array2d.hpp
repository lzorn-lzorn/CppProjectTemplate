#pragma once
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>
#include <string>
#include <functional>
#include <format>
namespace Tools {

template <typename Ty, class Allocator=std::allocator<Ty>>
class Array2D{
private:
    using real_array_type = std::vector<std::reference_wrapper<Ty>, Allocator>;
public:
    using value_type = real_array_type::value_type;
    using size_type = real_array_type::size_type;
    using allocator_type = real_array_type::allocator_type;
    

public:
    Array2D() 
        : row(1), col(0) {}
    Array2D(const uint32_t row, const uint32_t col) 
        : row(row), col(col) {}
    Array2D(const Array2D&) = default;
    Array2D(Array2D&&) = default;
    Array2D& operator=(const Array2D&) = default;
    Array2D& operator=(Array2D&&) = default;
    ~Array2D() = default;

    bool operator==(const Array2D& other) const{
        return array1d == other.array1d && row == other.row && col == other.col;
    }

    bool operator!=(const Array2D& other) const{
        return !(*this == other);
    }

    std::vector<Ty> operator[](const uint32_t row) const noexcept{
        std::vector<Ty> row_ {};
        row_.reserve(col);
        for (uint32_t col = 0; col < this->col; ++col) {
            row_.emplace_back(array1d[row * this->col + col]);
        }
        return row_;
    }
    std::vector<Ty> GetRow(const uint32_t row) const {
        if (row >= this->row) {
            std::string err_msg = std::format("Row index out of bounds: row={}, this->row={}", row, this->row);
            throw std::out_of_range(err_msg);
        }
        return operator[](row);
    }
    value_type At(const uint32_t row, const uint32_t col) const {
        if (IsOutOfBoundary(row, col)){
            std::string err_msg = std::format(
                "Index out of bounds: row={}, col={}, this->row={}, this->col={}, Index = row * this->col + col = {}, boundary is {}",
                row, col, this->row, this->col, row * this->col + col, array1d.size()
            );
            throw std::out_of_range(err_msg);
        }
        return array1d[row * this->col + col];
    }
public:


private:
    struct Coordinate {
        uint64_t x, y;
    };
    bool IsOutOfBoundary(const uint32_t row, const uint32_t col) const noexcept{
        return !(row < this->row && col < this->col && (array1d.size() > (row * this->col + col)));
    }
    uint64_t GetIndex(const uint32_t row, const uint32_t col) const noexcept {
        return row * this->col + col;
    }
    Coordinate GetXY(const uint64_t num) const noexcept {
        return { num / col, num % col };
    }

private:
    std::vector<std::reference_wrapper<Ty>, Allocator> array1d;
    uint32_t row{ 1 }, col{ 0 };
};

}