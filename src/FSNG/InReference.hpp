#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>

#include "nlohmann/json.hpp"

namespace FSNG {

struct InReference {
    InReference() = default;
    InReference(void const *data, int const size, std::type_info const *info) : data(data), size(size), info(info) {};

    // std::vector
    template<typename T>
    requires std::same_as<T, std::vector<typename T::value_type>>
    InReference(T const &t) : data(static_cast<void const*>(t.data())),
                              size(t.size()*sizeof(typename T::value_type)),
                              info(&typeid(int)),
                              isStandardLayout(std::is_standard_layout<T>::value) {};
    // std::string
    template<typename T>
    requires std::same_as<T, std::string>
    InReference(T const &t) : data(static_cast<void const*>(t.data())),
                              size(t.size()),
                              info(&typeid(char*)),
                              isStandardLayout(std::is_standard_layout<T>::value) {};
    // char array
    template <std::size_t N>
    InReference(char const (&t)[N]) : data(static_cast<void const*>(&t)),
                                      size(N-1),
                                      info(&typeid(char*)),
                                      isStandardLayout(true) {};
    // array of fundamental type
    template <typename T, std::size_t N>
    requires std::is_fundamental_v<T>
    InReference(T const (&t)[N]) : data(static_cast<void const*>(&t)),
                                   size(N*sizeof(T)),
                                   info(&typeid(T)),
                                   isStandardLayout(std::is_standard_layout<T>::value) {};
    // other types
    template<typename T>
    InReference(T const &t) : data(static_cast<void const*>(&t)),
                              size(sizeof(T)),
                              info(&typeid(T)),
                              isStandardLayout(std::is_standard_layout<T>::value) {};

    void const *data = nullptr;
    unsigned int size = 0;
    std::type_info const *info = nullptr;
    bool isStandardLayout = false;
};
} 