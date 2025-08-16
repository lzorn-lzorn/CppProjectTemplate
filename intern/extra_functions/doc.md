
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