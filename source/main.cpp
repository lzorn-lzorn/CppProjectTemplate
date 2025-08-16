
#include "../intern/extra_functions/survival_time.hpp"
#include <iostream>
#include <syncstream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>

template <template<typename> typename Extra>
class MyClass : public Extra<MyClass<Extra>> {
public:
    using Extra<MyClass<Extra>>::ExtraElapsed;
    using Extra<MyClass<Extra>>::ExtraStartTimekeeping;
    using Extra<MyClass<Extra>>::ExtraStopTimekeeping;

    // 构造参数转发
    template<typename... Args>
    MyClass(Args&&... args)
        : Extra<MyClass<Extra>>(std::forward<Args>(args)...) {}
};

template <template<typename> typename Extra>
class MyClass2 : public Extra<MyClass<Extra>> {
public:
    using Extra<MyClass<Extra>>::ExtraElapsed;
    using Extra<MyClass<Extra>>::ExtraStartTimekeeping;
    using Extra<MyClass<Extra>>::ExtraStopTimekeeping;

    // 构造参数转发
    MyClass2(bool is_start_timekeeping)
        : Extra<MyClass<Extra>>(is_start_timekeeping) {}
};

template <template<typename> typename Extra>
class MyClass3 : public Extra<MyClass2<Extra>> {
public:
    using Extra<MyClass2<Extra>>::ExtraElapsed;
    using Extra<MyClass2<Extra>>::ExtraStartTimekeeping;
    using Extra<MyClass2<Extra>>::ExtraStopTimekeeping;

    template<typename... Args>
    MyClass3(Args&&... args)
        : Extra<MyClass2<Extra>>(std::forward<Args>(args)...) {}
};

void thread_func(int id, double sleep_before, double sleep_during, double sleep_after, std::vector<double>& results) {
    MyClass<Extra::SurvivalTime> obj{false};
    std::this_thread::sleep_for(std::chrono::duration<double>(sleep_before));
    obj.ExtraStartTimekeeping();
    std::this_thread::sleep_for(std::chrono::duration<double>(sleep_during));
    obj.ExtraStopTimekeeping();
    std::this_thread::sleep_for(std::chrono::duration<double>(sleep_after));
    results[id] = obj.ExtraElapsed();
}

int main() {
    using Extra::SurvivalTime;
    {
        MyClass<SurvivalTime> obj;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Elapsed time1: " << std::fixed << std::setprecision(6) 
                << obj.ExtraElapsed() << " seconds" << std::endl;
    }
    
    {
        MyClass<SurvivalTime> obj{false};
        std::this_thread::sleep_for(std::chrono::seconds(2));
        obj.ExtraStartTimekeeping();
        std::this_thread::sleep_for(std::chrono::seconds(4));
        obj.ExtraStopTimekeeping();
        std::this_thread::sleep_for(std::chrono::seconds(6));
        std::cout << "Elapsed time2: " << std::fixed << std::setprecision(6) 
                << obj.ExtraElapsed() << " seconds" << std::endl;
    }

    {
        MyClass2<SurvivalTime> obj{false};
        std::this_thread::sleep_for(std::chrono::seconds(2));
        obj.ExtraStartTimekeeping();
        std::this_thread::sleep_for(std::chrono::seconds(4));
        obj.ExtraStopTimekeeping();
        std::this_thread::sleep_for(std::chrono::seconds(6));
        std::cout << "Elapsed time2 with param: " << std::fixed << std::setprecision(6) 
                << obj.ExtraElapsed() << " seconds" << std::endl;
    }

    {
        MyClass3<SurvivalTime> obj{false};
        std::this_thread::sleep_for(std::chrono::seconds(2));
        obj.ExtraStartTimekeeping();
        std::this_thread::sleep_for(std::chrono::seconds(4));
        obj.ExtraStopTimekeeping();
        std::this_thread::sleep_for(std::chrono::seconds(6));
        std::cout << "Elapsed time2 with forward params: " << std::fixed << std::setprecision(6) 
                << obj.ExtraElapsed() << " seconds" << std::endl;
    }

    {
        constexpr int thread_count = 4;
        std::vector<double> results(thread_count, 0.0);
        std::vector<std::thread> threads;

        std::cout << "Multi-threaded SurvivalTime test started...\n";
        for (int i = 0; i < thread_count; ++i) {
            double before = 0.5 * i;   // 每个线程启动前等待时间不同
            double during = 1.5 + 0.5 * i; // 计时长度不同
            double after = 0.2 * i;    // 停止后等待时间不同
            threads.emplace_back(thread_func, i, before, during, after, std::ref(results));
        }
        for (auto& th : threads) th.join();

        for (int i = 0; i < thread_count; ++i) {
            std::osyncstream(std::cout) << "Thread " << i << " measured: " << std::fixed << std::setprecision(6)
                    << results[i] << " seconds\n";
        }
        std::osyncstream(std::cout) << "Multi-threaded test finished.\n";
    }
    return 0;
}