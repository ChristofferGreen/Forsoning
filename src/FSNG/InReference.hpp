#pragma once
#include <unordered_map>
#include <vector>
#include <string>

#include "nlohmann/json.hpp"

namespace FSNG {

// std::vector
template<typename T>                     struct is_std_vector                            {static constexpr bool value = false;};
template<typename T, typename Allocator> struct is_std_vector<std::vector<T, Allocator>> {static constexpr bool value = true;};

// char array
template<typename T>    struct is_char_array          : std::false_type {};
template<std::size_t N> struct is_char_array<char[N]> : std::true_type {};

// Data
template<typename T> inline auto data_pointer(T& t)                        -> void const*;
template<>           inline auto data_pointer<std::string>(std::string& t) -> void const* {return static_cast<void const*>(t.data());}
template<typename T> inline auto data_pointer(T& t)                        -> void const* {return static_cast<void const*>(&t);}

// Size
template<typename T> inline auto data_size(T& t)                        -> unsigned int;
template<>           inline auto data_size<std::string>(std::string& t) -> unsigned int {return t.size();}
template<typename T> inline auto data_size(T& t)                        -> unsigned int {return sizeof(T);}

//    InReference(T const &t) : data(std::is_same_v<T, std::string> ? t.data() : &t),
  //                            size(std::is_same_v<T, std::string> ? t.size() : sizeof(T)),
struct InReference {
    InReference() = default;
    InReference(void const *data, int const size, std::type_info const *info) : data(data), size(size), info(info) {};
    template<typename T>
    InReference(T const &t) : data(data_pointer(t)),
                              size(data_size(t)),
                              info(is_char_array<T>::value ? &typeid(char*) : &typeid(T)),
                              isStandardLayout(std::is_same_v<T, std::string> ? true : std::is_standard_layout<T>::value),
                              isStdVector(is_std_vector<T>::value),
                              isFundamentalType(std::is_fundamental<T>::value) {};

    void const *data = nullptr;
    unsigned int size = 0;
    std::type_info const *info = nullptr;
    bool isStandardLayout = false;
    bool isStdVector = false;
    bool isFundamentalType = false;
};
} 