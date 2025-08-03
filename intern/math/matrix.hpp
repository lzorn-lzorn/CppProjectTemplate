#include <cmath>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <algorithm>
#include <thread>

namespace Math {
    template <typename Tp>
    concept Arithmetic = requires {
        requires std::is_arithmetic_v<Tp>;
        requires !std::is_same_v <std::remove_cvref_t<Tp>, bool>;
        requires !std::is_same_v <std::remove_cvref_t<Tp>, char>;
    };
    template <Arithmetic Tp>
    class Matrix{
    private:
        std::vector<Tp> array;
        uint32_t row, col;
    public:
        Matrix(const uint32_t row, const uint32_t col) 
            : row(row), col(col) {}
        Matrix(const Matrix&) = default;
        Matrix(Matrix&&) = default;
        Matrix& operator=(const Matrix&) = default;
        Matrix& operator=(Matrix&&) = default;
        ~Matrix() = default;
        bool operator==(const Matrix& other) const;
        Matrix& operator+(const Matrix& other) noexcept;
        Matrix& operator-(const Matrix& other) noexcept;
        Matrix& operator*(const Matrix& other) noexcept;
        Matrix& operator/(const Matrix& other);
        Matrix& operator+(const Tp& val) noexcept;
        Matrix& operator-(const Tp& val) noexcept;
        Matrix& operator*(const Tp& val) noexcept;
        Matrix& operator/(const Tp& val);
    public:
        Tp& Get(const uint32_t x, const uint32_t y);
        void Set(const uint32_t x, const uint32_t y, const Tp val);
        uint32_t Row() const;
        uint32_t Col() const;
        
    public: 
        double Det() const;
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
    };
}