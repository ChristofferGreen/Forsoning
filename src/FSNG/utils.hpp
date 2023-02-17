#pragma once
#include <functional>
#include <random>
#include <string>
#include <iostream>
#include <mutex>
#include <thread>
#include <shared_mutex>

namespace FSNG {

inline auto copy_byte_back_insert(auto start, auto nbr, auto &to) {
    std::copy(static_cast<std::byte const * const>(static_cast<void const * const>(start)),
              static_cast<std::byte const * const>(static_cast<void const * const>(start)) + nbr,
              std::back_inserter(to));
}

inline auto copy_byte_raw(std::byte* start, int const nbr, std::byte* to) {
    std::copy(start, start + nbr, to);
}

template <typename T>
constexpr auto is_fundamental_type() -> bool {
return typeid(T)==typeid(bool)               ||
       typeid(T)==typeid(signed char)        ||
       typeid(T)==typeid(unsigned char)      ||
       typeid(T)==typeid(wchar_t)            ||
       typeid(T)==typeid(short)              ||
       typeid(T)==typeid(unsigned short)     ||
       typeid(T)==typeid(int)                ||
       typeid(T)==typeid(unsigned int)       ||
       typeid(T)==typeid(long)               ||
       typeid(T)==typeid(unsigned long)      ||
       typeid(T)==typeid(long long)          ||
       typeid(T)==typeid(unsigned long long) ||
       typeid(T)==typeid(double)             ||
       typeid(T)==typeid(long double);
}

inline auto random_string(std::string::size_type length = 32) -> std::string {
    static auto& chrs = "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    thread_local static std::mt19937 rg{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

    std::string s;
    s.reserve(length);

    while(length--)
        s += chrs[pick(rg)];

    return s;
}

inline auto random_number(int min, int max) -> int {
  static std::mt19937 rnd(std::time(nullptr));
  return std::uniform_int_distribution<>(min,max)(rnd);
}

inline auto printm(std::string const &s) -> void {
    static std::shared_mutex printMutex;
    auto writeLock = std::unique_lock<std::shared_mutex>(printMutex);
    std::cout << s << std::endl;
}

inline auto sleep_thread_ms(int const ms) {
    auto const secondInNanoseconds = 1000000000;
    std::this_thread::sleep_for(std::chrono::nanoseconds(secondInNanoseconds*ms));
}

inline auto sleep_thread(float from=0.05, float to=0.3) {
    auto const time_ms = random_number(from, to);
    sleep_thread_ms(time_ms);
}

}

#ifdef __clang__
#define STD_EXPERIMENTAL std::experimental
#include <experimental/coroutine>
#else
#define STD_EXPERIMENTAL std
#include <coroutine>
#endif