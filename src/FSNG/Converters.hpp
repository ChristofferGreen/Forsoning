#pragma once
#include <unordered_map>

#include "nlohmann/json.hpp"

namespace FSNG {
struct Converters {
    inline static std::unordered_map<std::type_info const*, std::function<nlohmann::json(std::byte const*, int const size)>> toJSONConverters;
    inline static std::unordered_map<std::type_info const*, std::function<void(std::vector<std::byte> &vec, void const *obj)>> toByteArrayConverters;
};
}