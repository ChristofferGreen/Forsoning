#pragma once
#include <functional>
#include <unordered_map>

#include "nlohmann/json.hpp"

namespace FSNG {
namespace Converters {
inline std::unordered_map<std::type_info const*, std::function<nlohmann::json(std::byte const *fromBytes)>>                           triviallyCopyableToJSON;
inline std::unordered_map<std::type_info const*, std::function<nlohmann::json(std::byte const *fromBytes, int size)>>                 toJSON;
inline std::unordered_map<std::type_info const*, std::function<void          (std::vector<std::byte> &vec, void const *obj)>>         toByteArray;
inline std::unordered_map<std::type_info const*, std::function<void          (std::vector<std::byte> &vec, void const *obj)>>         toCompressedByteArray;

inline std::unordered_map<std::type_info const*, std::function<bool          (std::byte const *fromBytes, int size, void *toObject)>> fromJSON;
inline std::unordered_map<std::type_info const*, std::function<bool          (std::byte const *fromBytes, void *toObj)>>              fromByteArray;
inline std::unordered_map<std::type_info const*, std::function<bool          (std::byte const *fromBytes, void *toObj)>>              fromCompressedByteArray;
};
}
