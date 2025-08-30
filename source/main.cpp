#include <iostream>
#include <thread>
#include "../Intern/Print/Print.hpp"
int main() {
    using namespace Tools;

    Printable p;

    // 设置输出流
    p.SetStream(std::cout);

    // 设置时间
    auto now = std::chrono::system_clock::now();
    p.SetTime(now);

    // 设置线程ID
    p.SetThreadId(std::this_thread::get_id());

    // 测试 operator()
    std::cout << "Testing operator():\n";
    p(std::cout, "Extra argument1", 42, 3.14);

    // 测试 Message()
    std::cout << "\nTesting Message():\n";
    std::string msg = p.Message("Extra arg2", 123, "hello");
    std::cout << msg << std::endl;

    // 只设置部分成员
    Printable p2;
    p2.SetStream(std::cout);
    p2.SetTime(now);
    std::cout << "\nTesting Message() (no thread id):\n";
    std::cout << p2.Message() << std::endl;

    Printable p3;
    p3.SetStream(std::cout);
    p3.SetThreadId(std::this_thread::get_id());
    std::cout << "\nTesting Message() (no time):\n";
    std::cout << p3.Message() << std::endl;

    Printable p4;
    p4.SetStream(std::cout);
    std::cout << "\nTesting Message() (no thread id and no time):\n";
    std::cout << p4.Message() << std::endl;

    return 0;
}
