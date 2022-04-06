#pragma once
#include <unordered_map>

#include "nlohmann/json.hpp"

namespace FSNG {
struct InReference {
    void const *data = nullptr;
    int size = 0;
    std::type_info const *info = nullptr;
    inline static std::unordered_map<std::type_info const*, std::function<nlohmann::json(std::byte const*)>> converters; 
};
}