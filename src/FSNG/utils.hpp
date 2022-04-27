#pragma once
#include "FSNG/Coroutine.hpp"
#include "FSNG/Data.hpp"

#include <functional>

namespace FSNG {
namespace Converters {
inline static std::unordered_map<std::type_info const*, std::function<nlohmann::json(std::byte const*, int const size)>> toJSONConverters;
inline static std::unordered_map<std::type_info const*, std::function<void(std::vector<std::byte> &vec, void const *obj)>> toByteArrayConverters;
};

auto copy_byte_back_insert(auto start, auto nbr, auto &to) {
    std::copy(static_cast<std::byte const * const>(static_cast<void const * const>(start)),
              static_cast<std::byte const * const>(static_cast<void const * const>(start)) + nbr,
              std::back_inserter(to));
}
}