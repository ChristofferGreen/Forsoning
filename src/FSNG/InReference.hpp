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
template<typename T>    struct is_char_array                : std::false_type {};
template<std::size_t N> struct is_char_array<char const[N]> : std::true_type  {};

// Standard Layout
template<typename T>inline bool is_standard_layout             (const T& value)           {return std::is_standard_layout<T>::value;}
template<>          inline bool is_standard_layout<std::string>(const std::string& value) {return true;}

// Data
template<typename T> inline auto data_pointer(T const &t)                        -> void const*;
template<>           inline auto data_pointer<std::string>(std::string const &t) -> void const* {return static_cast<void const*>(t.data());}
template<typename T> inline auto data_pointer(T const &t)                        -> void const* {return static_cast<void const*>(&t);}

// Size
template<typename T> inline auto data_size(T const &t)                        -> unsigned int;
template<>           inline auto data_size<std::string>(std::string const &t) -> unsigned int {return t.size();}
template<size_t N>   inline auto data_size(char const (&t)[N])                -> unsigned int {return N - 1;}
template<typename T> inline auto data_size(T const &t)                        -> unsigned int {return sizeof(T);}

// Type
template<typename T> inline auto type_id(T const &t)                        -> std::type_info const*;
template<>           inline auto type_id<std::string>(std::string const &t) -> std::type_info const* {return &typeid(char*);}
template<size_t N>   inline auto type_id(char const (&t)[N])                -> std::type_info const* {return &typeid(char*);}
template<typename T> inline auto type_id(T const &t)                        -> std::type_info const* {return &typeid(T);}

struct InReference {
    InReference() = default;
    InReference(void const *data, int const size, std::type_info const *info) : data(data), size(size), info(info) {};
    template<typename T>
    InReference(T const &t) : data(data_pointer(t)),
                              size(data_size(t)),
                              info(type_id(t)),
                              isStandardLayout(is_standard_layout(t)),
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