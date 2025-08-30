#pragma once

#include <atomic>
#include <cstddef>
namespace Extra{
/**
    template <template<typename> typename Extra>
    class YourClass : public Extra<YourClass<Extra>> {};

    YourClass<Extra::ObjCounter>::GetCount()
*/
template <typename Ty>
class ObjCounter{
public:
    ObjCounter() noexcept {
        count.fetch_add(1);
    }

    ObjCounter(const ObjCounter&) noexcept {
        count.fetch_add(1);
    }

    ObjCounter(ObjCounter&&) noexcept {
        count.fetch_add(1);
    }

    ~ObjCounter() noexcept {
        count.fetch_sub(1);
    }

    ObjCounter& operator=(const ObjCounter&) noexcept = default;
    ObjCounter& operator=(ObjCounter&&) noexcept = default;

    static size_t GetCount() noexcept {
        return count.load();
    }
private:
    static inline std::atomic<size_t> count = 0;
};

}