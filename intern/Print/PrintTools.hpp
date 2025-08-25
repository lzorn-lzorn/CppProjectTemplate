#pragma once
#include <chrono>
#include <string>
#if defined(__cpp_lib_format)
  #include <format>
#endif
namespace Tools{
using namespace std::chrono;
inline std::string GetTimeToString(
    /* 年月日时分秒 */
#if defined(__cpp_lib_format)
    std::format_string<int, unsigned, unsigned, unsigned, unsigned, double> format,
#else
    const std::string& format,
#endif
    time_point<system_clock> time = system_clock::now(),
    int accuracy = 4

){
    // 1. 拆分年月日
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

/* 
 * @function: 打印单个值
 * @Supported types: 
 * @   BaseType: int, float, double, std::string, bool, const char*, enum
 * @   AdvancedType: std::optional, std::variant, std::any
 */
template <typename ValTy>
inline std::string ValToString(const ValTy& val){

}
}
