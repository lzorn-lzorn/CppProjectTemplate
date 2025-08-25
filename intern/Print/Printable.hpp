#include <chrono>
#include <optional>
#include <thread>
#include <ostream>
#include <string>
#include <sstream>
#include <functional>
#include "PrintTools.hpp"

namespace Tools{
using namespace std::chrono;
enum class Level{
    Normal, Warning, Error
};

class Printable{
public:
    Printable() = default;
    Printable(std::ostream& stream)
        : stream(&stream) {
            time = std::nullopt;
            threadId = std::nullopt;
        }
    ~Printable() = default;
    
    template <typename... Args>
    void operator()(std::ostream& stream, const Args&... args) const{
        if (!Check()) return ;
        stream << Message(args...);
    }

    template <typename... Args>
    std::string Message(const Args&... args) const{
        std::ostringstream oss;
        if (threadId) {
            oss << "[thread id: " << std::hash<std::thread::id>{}(*threadId) << "] ";
        }
        if (time) {
            oss << "[call time: " << GetTimeToString("{}-{}-{},{}-{}-{}" , *time) << "] ";
        }
        return oss.str();
    }
public:
    bool Check() const noexcept {
        return stream != nullptr;
    }
    Printable& SetStream(std::ostream& stream) noexcept{
        this->stream = &stream;
        return *this;
    }
    Printable& SetTime(time_point<system_clock> time) noexcept{
        this->time = time;
        return *this;
    }
    Printable& SetThreadId(std::thread::id id) noexcept{
        this->threadId = id;
        return *this;
    }

private:
    std::optional<time_point<system_clock>> time { std::nullopt };
    std::optional<std::thread::id> threadId { std::nullopt };
    std::ostream* stream { nullptr };
};



}
