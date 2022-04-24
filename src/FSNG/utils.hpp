#pragma once
#include "FSNG/Coroutine.hpp"
#include "FSNG/Data.hpp"

#include <functional>

namespace FSNG {
auto copy_byte_back_insert(auto start, auto nbr, auto &to) {
    std::copy(static_cast<std::byte const * const>(static_cast<void const * const>(start)),
              static_cast<std::byte const * const>(static_cast<void const * const>(start)) + nbr,
              std::back_inserter(to));
}
}