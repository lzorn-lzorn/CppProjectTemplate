
# Describe
这是一个基于CRTP的额外功能库, 其提供了一些通用的对象功能扩展, 具体使用方式为:
This library support some extra function by Curiously Recurring Template Pattern(CRTP).

# Object Counter
Object Counter 是一个对象数量计时器, 他可以帮助你计算你创建的类的数量, 同时他是线程安全的
There is a thread-safe ObjectCount, which can help count objects you create
## Usage
```Cpp
// Get one extra function
template <template<typename> typename Extra>
class YourClass : public Extra<YourClass<Extra>> {};

// Get more extra functions
template <template<typename...> typename... ExtraFunctions>
class YourClass : public ExtraFunctions<YourClass<ExtraFunctions...>>...{};
```


## Test
```Cpp
std::string now_time() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    std::osyncstream(ss) << std::put_time(std::localtime(&t), "%H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

int main() {
    std::vector<std::jthread> threads;
    for (int i = 0; i < 10; ++i) {
        std::osyncstream(std::cout) << "[Main] Before thread " << i << ": " << now_time() << std::endl;
        threads.emplace_back([i] {
            std::osyncstream(std::cout) << "[Thread " << i << "] Start: " << now_time() << std::endl;
            MyClass<Extra::ObjCounter> obj;
            std::osyncstream(std::cout) << "[Thread " << i << "] Obj constructed: " << now_time() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::osyncstream(std::cout) << "[Thread " << i << "] End: " << now_time() << std::endl;
        });

        std::osyncstream(std::cout) << "[Main] After thread " << i << ": " << now_time() << " Count: " << MyClass<Extra::ObjCounter>::GetCount() << std::endl;
    }
    // 线程自动 join
    std::osyncstream(std::cout) << "[Main] All threads started: " << now_time() << std::endl;
    std::osyncstream(std::cout) << "[Main] Final count: " << MyClass<Extra::ObjCounter>::GetCount() << std::endl;
    return 0;
}
```


# SurvivalTime
SurvivalTime 是一个线程安全的类存活时间的计时器, 他允许你随时随地地获取一个存活的类的已经存活时间
SurvivalTime is a thread-safe timer which can record your object live time. 
And check the time everwhere.

If you want get your object's survival time, you need get your class inherite from Class SurviveTime by CRTP.
- If you don't specially point out, the timer will start when your object construct.
- And the timer only be used one time! One `ExtraStartTimekeeping()` and One `ExtraStopTimekeeping()`.
- You can call `ExtraElapsed()` to get duration. If you don't start but call `ExtraElapsed()`, it will return -1.
 

## Test and Example
```Cpp

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
```