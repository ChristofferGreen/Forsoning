#pragma once
#include <unordered_map>

#include "nlohmann/json.hpp"

namespace FSNG {
struct InReference {
    InReference() = default;
    InReference(void const *data, int const size, std::type_info const *info) : data(data), size(size), info(info) {};
    template<typename T>
    InReference(T const &t) : data(&t),
                              size(sizeof(T)),
                              info(&typeid(T)),
                              isStandardLayout(std::is_standard_layout<T>::value),
                              isFundamentalType(std::is_fundamental<T>::value) {};

    void const *data = nullptr;
    int size = 0;
    std::type_info const *info = nullptr;
    bool isStandardLayout = false;
    bool isFundamentalType = false;
};
}