#pragma once

#include <string>

#include "Printable.hpp"
namespace Tools{
template <typename... Args>
void Print(std::ostream& stream, Level level, const Args&... args)
{
    Printable p;
    std::thread::id threadId = std::this_thread::get_id();
    const time_point<system_clock> now = std::chrono::system_clock::now();

    (stream << ... << args) << std::endl;
}
}
