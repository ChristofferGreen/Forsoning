#pragma once
#include "FSNG/Coroutine.hpp"
#include "FSNG/Data.hpp"

#include <functional>
#include <random>
#include <string>

namespace FSNG {
namespace Converters {
inline std::unordered_map<std::type_info const*, std::function<nlohmann::json(std::byte const *fromBytes, int size)>> toJSONConverters;
inline std::unordered_map<std::type_info const*, std::function<bool          (std::byte const *fromBytes, int size, void *toObject)>> fromJSONConverters;
inline std::unordered_map<std::type_info const*, std::function<void          (std::vector<std::byte> &vec, void const *obj)>> toByteArrayConverters;
inline std::unordered_map<std::type_info const*, std::function<bool          (std::byte const *fromBytes, void *toObj)>> fromByteArrayConverters;
};

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

}