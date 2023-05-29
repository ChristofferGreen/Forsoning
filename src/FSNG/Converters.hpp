#pragma once
#include <functional>
#include <unordered_map>

#include "nlohmann/json.hpp"

namespace FSNG {
namespace Converters {
inline std::unordered_map<std::type_info const*, std::function<nlohmann::json(std::byte const *fromBytes, int size)>> toJSONConverters;
inline std::unordered_map<std::type_info const*, std::function<bool          (std::byte const *fromBytes, int size, void *toObject)>> fromJSONConverters;
inline std::unordered_map<std::type_info const*, std::function<void          (std::vector<std::byte> &vec, void const *obj)>> toByteArrayConverters;
inline std::unordered_map<std::type_info const*, std::function<bool          (std::byte const *fromBytes, void *toObj)>> fromByteArrayConverters;
};
}
