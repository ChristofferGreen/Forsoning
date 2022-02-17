#pragma once
#include "FSNG/Codex.hpp"

#include <unordered_map>
#include <shared_mutex>

namespace FSNG {
struct CodicesAegis {
    CodicesAegis() = default;
    CodicesAegis(CodicesAegis const &other) : codices(other.codices) {}

    auto readMutex() const {
        return std::shared_lock<std::shared_mutex>(this->mutex);
    }

    auto writeMutex() const {
        return std::lock_guard<std::shared_mutex>(this->mutex);
    }

    std::unordered_map<std::string, Codex> codices;
    mutable std::shared_mutex mutex;
};
}