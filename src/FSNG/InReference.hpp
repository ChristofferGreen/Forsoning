#pragma once
#include <unordered_map>

#include "nlohmann/json.hpp"

namespace FSNG {
struct InReference {
    void const *data = nullptr;
    int size = 0;
    std::type_info const *info = nullptr;
    bool isTriviallyCopyable = false;
    inline static std::unordered_map<std::type_info const*, std::function<nlohmann::json(std::byte const*)>> toJSONConverters; 
    inline static std::unordered_map<std::type_info const*, std::function<void(std::vector<std::byte> &vec, void const *obj)>> toByteArrayConverters; 
    inline static std::unordered_map<std::type_info const*, std::function<void(std::byte const *vec, void *obj)>> fromByteArrayConverters; 
};
}