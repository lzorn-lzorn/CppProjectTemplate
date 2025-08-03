#include <cmath>
#include <cstdint>
#include <array>
#include <algorithm>
#include <thread>
#include "traits.hpp"
namespace Math {
    
template <typename Derived, Arithmetic Tp, size_t Row, size_t Col>
class MatrixBase{
public:
    using value_type = Tp;
    static constexpr size_t row = Row;
    static constexpr size_t col = Col;
};


template <Arithmetic Tp, size_t Row, size_t Col>
class Matrix : public MatrixBase<Matrix<Tp, Row, Col>, Tp, Row, Col>{
public:
    using base_type = MatrixBase<Matrix<Tp, Row, Col>, Tp, Row, Col>;
    using value_type = base_type::value_type;
    using matrix_type = Matrix<Tp, Row, Col>;
    using base_type::base_type;
    static constexpr size_t row = value_type::Row;
    static constexpr size_t col = value_type::Col;
public:
    
    Matrix() = default;
    Matrix(const Matrix&) = default;
    Matrix(Matrix&&) = default;
    Matrix& operator=(const Matrix&) = default;
    Matrix& operator=(Matrix&&) = default;
    ~Matrix() = default;
    bool operator==(const Matrix& other) const;

    template <Arithmetic Ty, size_t OtherRow, size_t OtherCol>
    Matrix<Tp, Row, OtherCol>& operator*(const Matrix<Ty, OtherRow, OtherCol>& other) noexcept;
    matrix_type& operator-(const matrix_type& other) noexcept;
    matrix_type& operator+(const matrix_type& other) noexcept;

    matrix_type& operator+(const Tp& val) noexcept;
    matrix_type& operator-(const Tp& val) noexcept;
    matrix_type& operator*(const Tp& val) noexcept;
    matrix_type& operator/(const Tp& val);
public:
    Tp& Get(const uint32_t x, const uint32_t y);
    void Set(const uint32_t x, const uint32_t y, const Tp val);
    
public: 

    template <class Method>
    void TransformOne(const uint32_t x, const uint32_t y, Method transform);
    template <class Method>
    void TransformRow(const uint32_t index, Method transform);
    template <class Method>
    void TransformCol(const uint32_t index, Method transform);
    template <class Method>
    void TransformAll(Method transform);

public:
    void print() const noexcept;

private:    
    std::array<Tp, Row*Col> array;
};

template <Arithmetic Tp, size_t N>
class Matrix<Tp, N, N>{

};
}