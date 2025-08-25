#pragma once
#include <type_traits>
#include <string>
#include <memory>
#if defined(__cpp_lib_concepts)
#include <concepts>
#endif
#include <Proj.hpp>

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

#if defined (__cpp_lib_concepts)
// C++20 concepts 版本
// @=================C++20 is_convertible_to_string============== 
template <typename T>
concept is_convertible_to_string =
requires (T t) {
    { std::string(t) } -> std::same_as<std::string>;
};
// @==========================C++20 is_printable======================= 

template <typename Ty>
concept is_printable = requires (std::ostream& os,  const Ty& ty){
    { os << ty } -> std::same_as <std::ostream&>;
};

// @=======================C++20 pointer_extractor====================
template <typename Ty>
struct pointer_extractor {
    using value_type = Ty;
    static const void* get_ptr(const Ty& val) {
        return static_cast<const void*>(std::addressof(val));
    }
    static const value_type& deref(const Ty& val){
        return val;
    }
};

/* 处理裸指针 */
template <typename Ty> requires std::is_pointer_v<Ty>
struct pointer_extractor<Ty> {
    using value_type = typename pointer_extractor<std::remove_pointer_t<Ty>>::value_type;
    static const void* get_ptr(const Ty& ptr){
        if (ptr == nullptr)
            return nullptr;
        return pointer_extractor<std::remove_pointer_t<Ty>>::get_ptr(*ptr);
    }
    static const value_type& deref(const Ty& ptr){
        return pointer_extractor<std::remove_pointer_t<Ty>>::deref(*ptr);
    }
};

/* 处理标准智能指针 */ 
template <typename Ty>
requires is_smart_ptr<Ty>::value
struct pointer_extractor<Ty> {
    using InnerPtr = decltype(std::declval<Ty>().get());
    using value_type = typename pointer_extractor<InnerPtr>::value_type;
    static const void* get_ptr(const Ty& ptr) {
        if (ptr.get() == nullptr)
            return nullptr;
        return pointer_extractor<InnerPtr>::get_ptr(ptr.get());
    }
    static const value_type& deref(const Ty& ptr) {
        return pointer_extractor<InnerPtr>::deref(ptr.get());
    }
};
#else
/* C++11 SFINAE 版本 */ 
// @=================C++11 is_convertible_to_string============== 
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


// @==========================C++11 is_printable======================= 
template <typename Ty>
struct is_printable {
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

// @=======================C++11 pointer_extractor====================
template <typename Ty, 
          bool IsPointer = std::is_pointer<Ty>::value,
          bool IsUnique = is_unique_ptr<Ty>::value,
          bool IsShared = is_shared_ptr<Ty>::value,
          bool IsWeak = is_weak_ptr<Ty>::value>
struct pointer_extractor {
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
struct pointer_extractor<Ty, true, IsUnique, IsShared, IsWeak> {
    typedef typename pointer_extractor<typename std::remove_pointer<Ty>::type>::value_type value_type;
    static const void* get_ptr(const Ty& ptr) {
        if (ptr == nullptr)
            return nullptr;
        return pointer_extractor<typename std::remove_pointer<Ty>::type>::get_ptr(*ptr);
    }
    static const value_type& deref(const Ty& ptr){
        return pointer_extractor<typename std::remove_pointer<Ty>::type>::deref(*ptr);
    }
};

// unique_ptr特化
template <typename Ty, bool IsPointer, bool IsShared, bool IsWeak>
struct pointer_extractor<Ty, IsPointer, true, IsShared, IsWeak> {
    typedef decltype(std::declval<Ty>().get()) InnerPtr;
    // typedef InnerPtr value_type;
    typedef typename pointer_extractor<InnerPtr>::value_type value_type;
    static const void* get_ptr(const Ty& ptr) {
        if (ptr.get() == nullptr)
            return nullptr;
        return pointer_extractor<InnerPtr>::get_ptr(ptr.get());
    }
    static const value_type& deref(const Ty& ptr){
        return pointer_extractor<InnerPtr>::deref(ptr.get());
    }
};
// shared_ptr特化
template <typename Ty, bool IsPointer, bool IsUnique, bool IsWeak>
struct pointer_extractor<Ty, IsPointer, IsUnique, true, IsWeak> {
    typedef decltype(std::declval<Ty>().get()) InnerPtr;
    // typedef InnerPtr value_type;
    typedef typename pointer_extractor<InnerPtr>::value_type value_type;
    static const void* get_ptr(const Ty& ptr) {
        if (ptr.get() == nullptr)
            return nullptr;
        return pointer_extractor<InnerPtr>::get_ptr(ptr.get());
    }
    static const value_type& deref(const Ty& ptr){
        return pointer_extractor<InnerPtr>::deref(ptr.get());
    }
};
// weak_ptr特化
template <typename Ty, bool IsPointer, bool IsUnique, bool IsShared>
struct pointer_extractor<Ty, IsPointer, IsUnique, IsShared, true> {
    typedef decltype(std::declval<Ty>().lock()) InnerPtr;
    // typedef InnerPtr value_type;
    typedef typename pointer_extractor<InnerPtr>::value_type value_type;
    static const void* get_ptr(const Ty& ptr) {
        auto sp = ptr.lock();
        if (!sp)
            return nullptr;
        return pointer_extractor<InnerPtr>::get_ptr(sp);
    }
    static const value_type& deref(const Ty& ptr){
        auto sp = ptr.lock();
        return pointer_extractor<InnerPtr>::deref(sp);
    }
};
#endif
