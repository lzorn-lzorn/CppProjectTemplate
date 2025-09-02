#pragma once
#include <chrono>
#include <ostream>
#include <utility>
#include <tuple>
#include <any>
#include <string>
#include <type_traits>
#include "../Base/TypeTraits.hpp"
#if defined(__cpp_lib_format)
  #include <format>
#endif
#include "../Base/Proj.hpp"
namespace Tools{
using namespace std::chrono;
/*
 * @function: 将某个时间点转换为字符串
 * @param: format 打印格式, 年月日时分秒 "{}-{}-{}: {}-{}-{}" ms 在s后以小数形式表示
 * @param: time 时间点, 默认为当前时间
 * @param: accuracy ms精度, 默认为4位小数
 */
INLINE std::string GetTimeToString(
    /* 年月日时分秒 */
#if defined(__cpp_lib_format)
    std::format_string<int, unsigned, unsigned, unsigned, unsigned, double> format,
#else
    const std::string& format,
#endif
    time_point<system_clock> time = system_clock::now(),
    int accuracy = 4
){    // 1. 拆分年月日
    auto days_ = floor<days>(time);
    year_month_day ymd{days_};
    // 2. 拆分时分秒
    auto time_of_day = time - days_;
    auto hours_ = duration_cast<hours>(time_of_day);
    auto minutes_ = duration_cast<minutes>(time_of_day - hours_);
    auto seconds_ = duration_cast<seconds>(time_of_day - hours_ - minutes_);
    auto subseconds_ = duration_cast<microseconds>(time_of_day - hours_ - minutes_ - seconds_).count() / 1e6;
    double truncated = static_cast<long long>(subseconds_ * 10000) / std::pow(10.0, accuracy);

    // 3. 格式化输出
#if defined(__cpp_lib_format)
    return std::format(
        format, 
        int(ymd.year()),
        unsigned(ymd.month()),
        unsigned(ymd.day()),
        unsigned(hours_.count()),
        unsigned(minutes_.count()),
        (double)seconds_.count() + truncated
    );
#else
    char buffer[128];
    std::snprintf(
        buffer, sizeof(buffer),
        format.c_str(),
        int(ymd.year()),
        unsigned(ymd.month()),
        unsigned(ymd.day()),
        unsigned(hours_.count()),
        unsigned(minutes_.count()),
        (double)seconds_.count() + truncated
    );
    return std::string(buffer);
#endif
}


#if defined (__cpp_if_constexpr)
/* 
 * @function: 打印单个值
 * @Supported types: 
 * @   BaseType: int, float, double, std::string, bool, const char*, enum
 * @   AdvancedType: std::optional, std::variant, std::any
 */
template <typename ValTy>
INLINE std::string ValToString(const ValTy& val){
    using TargetType = std::remove_cvref_t<ValTy>;
    /* number */
    if constexpr (std::is_arithmetic_v<TargetType>){
        return std::to_string(val);
    /* std::string */
    } else if constexpr (std::is_same_v<std::string, TargetType>){
        return val;
    /* const char* */
    } else if constexpr (std::is_same_v<char*, TargetType>){
        return std::string(val);
    } else {
        return "";
    }
}

/* 
 * @function: 将一个指针类型的range转换为字符串
 * @note: 这个函数不会认为改指针是一个范围而只是一个变量
 */
template <typename PointTy>
INLINE std::string PtrToString(PointTy* ptr){
    using extractor = extract_pointer<PointTy>;
    using pointer = typename extractor::value_type;
    using target_type = typename std::remove_cvref<PointTy>::type;

    if constexpr (std::is_pointer<target_type>::value || is_smart_ptr<target_type>::value) {
        const void * raw_ptr = extractor::get_ptr(ptr);
        if (!raw_ptr){
            return "nullptr";
        }
        const pointer& deref_val = extractor::deref(ptr);

        /* 函数指针 */
        if constexpr (std::is_invocable<pointer>::value){
            return std::to_string(std::invoke(deref_val));
        } else {
        

        }
    }
    return "";
}

/*
 * @function: 将一个范围转换为字符串
 * @return: "val1, val2, val3, ... "
 */
template <typename Range>
INLINE std::string RangeToString(const Range& range){
    if constexpr (has_iterator<Range>::value){
        std::string res = "";
        for (const auto& item : range) {
            if constexpr (std::is_scalar<decltype(item)>::value) {
                res += ValToString(item) + ", ";
            } else if constexpr (is_ostreamable<decltype(item)>::value){
                std::ostringstream ss;
                ss << item;
                res += ss.str() + ", ";
            }
        }
        // 移除最后 ", "
        res.pop_back(); 
        res.pop_back(); 
        return res;
    }
    return "";
}

template <typename Ty>
INLINE std::string RangeToString(const Ty* arr, size_t size){

}

template <typename Ty>
INLINE std::string OptionToString(const std::optional<Ty>& opt){

}

template <typename Ty>
INLINE std::string AnyToString(const std::any& val){

}

template <typename Ty>
INLINE std::string VariantToString(const std::variant<Ty>& var){

}

template <typename... Args>
INLINE std::string TupleToString(const std::tuple<Args...>& dict){

}

template <typename First, typename Second>
INLINE std::string PairToString(const std::pair<First, Second>& p){

}

template <typename Ty>
INLINE std::string AtomicToString(const std::atomic<Ty>& val){

}

#else // 没有 if constexpr 这个语法

#endif 
}