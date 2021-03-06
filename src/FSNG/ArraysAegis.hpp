#pragma once
#include "FSNG/ArrayAegis.hpp"
#include "FSNG/PathSpaceTE.hpp"

#include <deque>
#include <shared_mutex>
#include <variant>

namespace FSNG {
struct ArraysAegis {
    ArraysAegis() = default;
    ArraysAegis(ArraysAegis const &other) : map(other.map) {}

    auto count(auto const &str) const -> int{
        auto const mapReadMutex = this->readMutex();
        return this->map.count(str);
    }

    auto readMutex() const -> std::shared_lock<std::shared_mutex> {
        return std::shared_lock<std::shared_mutex>(this->mutex);
    }

    auto writeMutex() const -> std::lock_guard<std::shared_mutex> {
        return std::lock_guard<std::shared_mutex>(this->mutex);
    }

    using VarT = std::variant<std::deque<int>,
                              std::deque<double>,
                              std::deque<std::string>,
                              std::deque<PathSpaceTE>>;
    std::unordered_map<std::string, ArrayAegis<VarT>> map;
    mutable std::shared_mutex mutex;
};
}