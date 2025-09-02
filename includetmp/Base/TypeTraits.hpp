#pragma once

#include <type_traits>
#include <string>
#include <memory>
#include <iterator>
#include <tuple>

#include <utility>
#include "Proj.hpp"
#include "TypeTraitsImpl.hpp"


// simple remove_cvref for C++11 compatibility
template <typename T>
struct remove_cvref { typedef typename std::remove_cv<typename std::remove_reference<T>::type>::type type; };


// Public trait: strip reference and cv-qualifiers and delegate to impl
template <typename T>
struct is_single_unwrapped_value
    : TypeTraitsImpl::is_single_unwrapped_value_impl<typename remove_cvref<T>::type>
{};

// ---------- examples / static_asserts ----------
/*
static_assert(is_single_unwrapped_value<int>::value, "int is single value");
static_assert(is_single_unwrapped_value<const int>::value, "const int");
static_assert(is_single_unwrapped_value<int*>::value, "int* -> pointee int is single");
static_assert(is_single_unwrapped_value<std::unique_ptr<int>>::value, "unique_ptr<int>");
static_assert(is_single_unwrapped_value<std::shared_ptr<int>>::value, "shared_ptr<int>");
static_assert(is_single_unwrapped_value<std::weak_ptr<int>>::value, "weak_ptr<int>");
static_assert(is_single_unwrapped_value<std::atomic<int>>::value, "atomic<int>");
static_assert(is_single_unwrapped_value<std::string>::value, "string");
static_assert(is_single_unwrapped_value<const char*>::value, "const char*");
#if defined(SINGLE_UNWRAPPED_HAVE_STRING_VIEW)
static_assert(is_single_unwrapped_value<std::string_view>::value, "string_view");
#endif
enum E { A, B };
static_assert(is_single_unwrapped_value<E>::value, "enum");
*/




// @function: 检查是否存在 T::iterator 或 T::const_iterator
template <typename T>
struct has_iterator : std::integer_sequence<bool,
    TypeTraitsImpl::has_iterator_impl<T>::value || TypeTraitsImpl::has_const_iterator_impl<T>::value> {};

// --- is_iterable: 检测是否可以对类型使用 std::begin/std::end ---
// 使用 decltype(std::begin(std::declval<T&>())) 与 std::end(...)
template<typename T, typename = void>
struct is_iterable : std::false_type {};

template<typename T>
struct is_iterable<T, void_t<
    decltype(std::begin(std::declval<T&>())),
    decltype(std::end(std::declval<T&>()))
>> : std::true_type {};

// alias
template<typename T>
struct is_range : is_iterable<T> {};

// variable template for convenience (C++14+)
#if defined (__cpp_variable_templates)
template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;
template <typename T>
INLINE constexpr bool has_iterator_v = has_iterator<T>::value;
template<typename T>
INLINE constexpr bool is_iterable_v = is_iterable<T>::value;
template<typename T>
INLINE constexpr bool is_range_v = is_range<T>::value;
template <typename T>
inline constexpr bool is_single_unwrapped_value_v = is_single_unwrapped_value<T>::value;
#endif 


// 判断是否是标准智能指针
template <typename T>
struct is_unique_ptr : std::false_type {};
template <typename T, typename D>
struct is_unique_ptr<std::unique_ptr<T, D>> : std::true_type {};

template <typename T>
struct is_shared_ptr : std::false_type {};
template <typename T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

template <typename T>
struct is_weak_ptr : std::false_type {};
template <typename T>
struct is_weak_ptr<std::weak_ptr<T>> : std::true_type {};

template <typename T>
struct is_smart_ptr : std::integral_constant<bool,
    is_unique_ptr<T>::value ||
    is_shared_ptr<T>::value ||
    is_weak_ptr<T>::value>
{};

#if defined (__cpp_lib_optional)
#include <optional>
// @ 判断是否是optional 
template <typename T>
struct is_optional : std::false_type {};
template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};
#endif

#if defined (__cpp_lib_variant)
#include <variant>

// @ 判断是否是variant
template <typename T>
struct is_variant : std::false_type {};
template <typename... Types>
struct is_variant<std::variant<Types...>> : std::true_type {};
#endif


// @================= is_convertible_to_string ============== 
template <typename Ty>
struct is_convertible_to_string {
private:
    template <typename U>
    static auto test(int) -> decltype(
        std::string(std::declval<U>()),
        std::true_type()
    );

    template <typename>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<Ty>(0))::value;
};


// @========================== is_ostreamable ======================= 
template <typename Ty>
struct is_ostreamable {
    template <typename U>
    static auto test(int) -> decltype(
        std::declval<std::ostream&>() << std::declval<const Ty&>(),
        std::true_type()
    );

    template <typename>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<Ty>(0))::value;
};

// @======================= extract_pointer ====================
template <typename Ty, 
          bool IsPointer = std::is_pointer<Ty>::value,
          bool IsUnique = is_unique_ptr<Ty>::value,
          bool IsShared = is_shared_ptr<Ty>::value,
          bool IsWeak = is_weak_ptr<Ty>::value>
struct extract_pointer {
    typedef Ty value_type;
    static const void* get_ptr(const Ty& val){
        return static_cast<const void*>(std::addressof(val));
    }
    static const value_type& deref(const Ty& val){
        return val;
    }
};

// 指针特化
template <typename Ty, bool IsUnique, bool IsShared, bool IsWeak>
struct extract_pointer<Ty, true, IsUnique, IsShared, IsWeak> {
    typedef typename extract_pointer<typename std::remove_pointer<Ty>::type>::value_type value_type;
    static const void* get_ptr(const Ty& ptr) {
        if (ptr == nullptr)
            return nullptr;
        return extract_pointer<typename std::remove_pointer<Ty>::type>::get_ptr(*ptr);
    }
    static const value_type& deref(const Ty& ptr){
        return extract_pointer<typename std::remove_pointer<Ty>::type>::deref(*ptr);
    }
};

// unique_ptr特化
template <typename Ty, bool IsPointer, bool IsShared, bool IsWeak>
struct extract_pointer<Ty, IsPointer, true, IsShared, IsWeak> {
    typedef decltype(std::declval<Ty>().get()) InnerPtr;
    // typedef InnerPtr value_type;
    typedef typename extract_pointer<InnerPtr>::value_type value_type;
    static const void* get_ptr(const Ty& ptr) {
        if (ptr.get() == nullptr)
            return nullptr;
        return extract_pointer<InnerPtr>::get_ptr(ptr.get());
    }
    static const value_type& deref(const Ty& ptr){
        return extract_pointer<InnerPtr>::deref(ptr.get());
    }
};
// shared_ptr特化
template <typename Ty, bool IsPointer, bool IsUnique, bool IsWeak>
struct extract_pointer<Ty, IsPointer, IsUnique, true, IsWeak> {
    typedef decltype(std::declval<Ty>().get()) InnerPtr;
    // typedef InnerPtr value_type;
    typedef typename extract_pointer<InnerPtr>::value_type value_type;
    static const void* get_ptr(const Ty& ptr) {
        if (ptr.get() == nullptr)
            return nullptr;
        return extract_pointer<InnerPtr>::get_ptr(ptr.get());
    }
    static const value_type& deref(const Ty& ptr){
        return extract_pointer<InnerPtr>::deref(ptr.get());
    }
};
// weak_ptr特化
template <typename Ty, bool IsPointer, bool IsUnique, bool IsShared>
struct extract_pointer<Ty, IsPointer, IsUnique, IsShared, true> {
    typedef decltype(std::declval<Ty>().lock()) InnerPtr;
    // typedef InnerPtr value_type;
    typedef typename extract_pointer<InnerPtr>::value_type value_type;
    static const void* get_ptr(const Ty& ptr) {
        auto sp = ptr.lock();
        if (!sp)
            return nullptr;
        return extract_pointer<InnerPtr>::get_ptr(sp);
    }
    static const value_type& deref(const Ty& ptr){
        auto sp = ptr.lock();
        return extract_pointer<InnerPtr>::deref(sp);
    }
};

#if defined (__cpp_lib_concepts)
#include <concepts>
// C++20 concepts 版本
// @=================C++20 IsConvertibleToString============== 
template <typename T>
concept IsConvertibleToString =
requires (T t) {
    { std::string(t) } -> std::same_as<std::string>;
};
// @==========================C++20 is_ostreamable======================= 

template <typename Ty>
concept IsPrintable = requires (std::ostream& os,  const Ty& ty){
    { os << ty } -> std::same_as <std::ostream&>;
};

#endif
