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

/*
 * single unwrapped type(SU type): can directly transform to string
 *     - arithemetic: std::to_string()
 *     - bool: true -> "true"; false -> "false"
 *     - enum: magic_enum
 *     - pointer: native pointer / smart pointer (point to SU type) -> unref call self
 *     - null pointer: "nullptr"
 *     - string type: std::string, const char*, char[], char -> std::string
 *     - atomic ?
 */

template <typename Ty, typename = void>
struct ToString {};

template <>
struct ToString<bool, void>{
    static std::string Convert(bool value) {
        return value ? "true" : "false";
    }
};
    
template <>
struct ToString<char, void>{
    static std::string Convert(char value) {
        return std::string(1, value);
    }
};
template <>
struct ToString<std::string, void>{
    static std::string Convert(const std::string& value) {
        return value;
    }
};
template <>
struct ToString<const char*, void>{
    static std::string Convert(const char* value) {
        return value ? value : "nullptr";
    }
};

template <>
struct ToString<char*, void> {
    static std::string Convert(char* s) {
        if (!s) return std::string("nullptr");
        return std::string(s);
    }
};

template <typename Ty>
struct ToString<Ty, typename std::enable_if<
    std::is_arithmetic<Ty>::value &&
    !std::is_same<Ty, bool>::value &&
    !std::is_same<Ty, char>::value
>::type>
{
    static std::string Convert(Ty v) {
        // Use std::to_string which exists for arithmetic types (integrals/floats).
        return std::to_string(v);
    }
};


#if defined(__has_include)
#  if __has_include(<magic_enum.hpp>)
#    include <magic_enum.hpp>
#    define TOSTRING_HAVE_MAGIC_ENUM 1
#  endif
#endif
template <typename Ty>
struct ToString<Ty, typename std::enable_if<std::is_enum<Ty>::value>::type> {
    static std::string Convert(Ty v) {
#if defined(TOSTRING_HAVE_MAGIC_ENUM)
        auto sv = magic_enum::enum_name(v);
        if (!sv.empty()) return std::string(sv);
        // fallthrough to underlying numeric if name empty
#endif
        using Under = typename std::underlying_type<Ty>::type;
        return ToString<Under>::Convert(static_cast<Under>(v));
    }
};

// --- Raw pointer (T*) ---
// Exclude char pointers because they are handled above as strings.
template <typename Pointee>
struct ToString<Pointee*, typename std::enable_if<
    !std::is_same<Pointee, char>::value &&
    !std::is_same<Pointee, const char>::value
>::type>
{
    static std::string Convert(Pointee* p) {
        if (!p) return std::string("nullptr");
        // delegate to ToString of the pointee type (decay cv)
        using PT = typename std::remove_cv<Pointee>::type;
        return ToString<PT>::Convert(*p);
    }
};


// --- std::unique_ptr ---
template <typename U, typename D>
struct ToString<std::unique_ptr<U, D>, void> {
    static std::string Convert(const std::unique_ptr<U, D>& p) {
        if (!p) return std::string("nullptr");
        using PT = typename std::remove_cv<U>::type;
        return ToString<PT>::Convert(*p);
    }
};

// --- std::shared_ptr ---
template <typename U>
struct ToString<std::shared_ptr<U>, void> {
    static std::string Convert(const std::shared_ptr<U>& p) {
        if (!p) return std::string("nullptr");
        using PT = typename std::remove_cv<U>::type;
        return ToString<PT>::Convert(*p);
    }
};

// --- std::weak_ptr ---
template <typename U>
struct ToString<std::weak_ptr<U>, void> {
    static std::string Convert(const std::weak_ptr<U>& wp) {
        auto sp = wp.lock();
        if (!sp) return std::string("nullptr");
        using PT = typename std::remove_cv<U>::type;
        return ToString<PT>::Convert(*sp);
    }
};


// --- Convenience helper that deduces and calls the correct specialization ---
// Accepts cv/ref types and forwards appropriately.
template <typename T>
inline std::string ToStringValue(const T& v) {
    using Clean = typename std::decay<T>::type;
    return ToString<Clean>::Convert(v);
}

// Overload for pointer types where argument is pointer (deduced exactly)
template <typename T>
inline std::string ToStringValue(T* p) {
    // dispatch to pointer specialization directly
    using CleanPtr = typename std::remove_cv<T>::type;
    return ToString<CleanPtr*>::Convert(p);
}

#if defined(TOSTRING_HAVE_MAGIC_ENUM)
#  undef TOSTRING_HAVE_MAGIC_ENUM
#endif
}


/* test
enum class Color : int { Red = 1, Green = 2, Blue = 3 };

struct S {
    int x;
};
std::ostream& operator<<(std::ostream& os, const S& s){ return os << "S(" << s.x << ")"; }

int main() {
    using util::ToStringValue;
    int i = 42;
    std::cout << ToStringValue(i) << "\n";               // "42"
    double d = 3.14;
    std::cout << ToStringValue(d) << "\n";               // "3.140000"
    bool b = true;
    std::cout << util::ToString<bool>::toString(b) << "\n"; // "true"
    char c = 'A';
    std::cout << ToStringValue(c) << "\n";               // "A"
    const char* cs = "hello";
    std::cout << ToStringValue(cs) << "\n";              // "hello"
    const char* nullcs = nullptr;
    std::cout << ToStringValue(nullcs) << "\n";          // "nullptr"
    Color col = Color::Green;
    std::cout << ToStringValue(col) << "\n";             // "Green" if magic_enum available, otherwise "2"
    int* p = &i;
    std::cout << ToStringValue(p) << "\n";               // "42"
    int* np = nullptr;
    std::cout << ToStringValue(np) << "\n";              // "nullptr"
    auto up = std::make_unique<int>(99);
    std::cout << ToStringValue(up) << "\n";              // "99"
    std::shared_ptr<S> sp = std::make_shared<S>();
    sp->x = 5;
    std::cout << ToStringValue(sp) << "\n";              // "S(5)" (delegates to ToString<S> -> fallback to "<unprintable ...>" unless you specialize S)
    std::atomic<int> a{7};
    std::cout << ToStringValue(a) << "\n";               // "7"
    return 0;
}
*/