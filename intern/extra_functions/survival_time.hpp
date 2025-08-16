#pragma once

#include <chrono>
#include <atomic>
namespace Extra {
    template <typename Ty>
    class SurvivalTime {
    public:
        SurvivalTime() noexcept
            : is_started_(false), is_stopped_(false), running_(false), elapsed_time_(0.0)
        {
            StartTimekeeping();
        }

        SurvivalTime(bool is_start_now) noexcept
            : is_started_(is_start_now), is_stopped_(false), running_(false), elapsed_time_(0.0)
        {
            if (is_start_now) {
                StartTimekeeping();
            }
        }

        // 禁止拷贝和赋值
        SurvivalTime(const SurvivalTime&) = delete;
        SurvivalTime& operator=(const SurvivalTime&) = delete;

        // 析构时自动停止计时
        ~SurvivalTime() noexcept {
            StopTimekeeping();
        }

        // 手动开始计时（只能调用一次）
        void ExtraStartTimekeeping() noexcept {
            bool expected = false;
            if (is_started_.compare_exchange_strong(expected, true)) {
                start_time_ = std::chrono::steady_clock::now();
                running_ = true;
            }
        }

        // 手动停止计时（只能调用一次）
        void ExtraStopTimekeeping() noexcept {
            bool expected = false;
            if (is_stopped_.compare_exchange_strong(expected, true)) {
                if (running_) {
                    auto end_time = std::chrono::steady_clock::now();
                    elapsed_time_ = std::chrono::duration<double>(end_time - start_time_);
                    running_ = false;
                }
            }
        }

        // 返回计时秒数
        double ExtraElapsed() const noexcept {
            if (running_) {
                auto now = std::chrono::steady_clock::now();
                return std::chrono::duration<double>(now - start_time_).count();
            } else {
                if (is_started_) {
                    return elapsed_time_.count();
                }else{
                    return -1;
                }
            }
        }

    private:
        std::atomic<bool> is_started_;
        std::atomic<bool> is_stopped_;
        std::chrono::time_point<std::chrono::steady_clock> start_time_;
        std::chrono::duration<double> elapsed_time_;
        std::atomic<bool> running_;
    };

}