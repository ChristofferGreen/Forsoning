#pragma once
#include "FSNG/Codex.hpp"

#include <unordered_map>
#include <shared_mutex>

namespace FSNG {
struct CodicesAegis {
    CodicesAegis() = default;
    CodicesAegis(CodicesAegis const &other) : codices(other.codices) {}

    auto write(auto const &fun) -> void {
        auto const mapWriteMutex = std::lock_guard<std::shared_mutex>(this->mutex);
        fun(this->codices);
    }

    auto read(auto const &fun) const -> void {
        auto const mapReadMutex = std::shared_lock<std::shared_mutex>(this->mutex);
        fun(this->codices);
    }
    
    private:
        std::unordered_map<std::string, Codex> codices;
        mutable std::shared_mutex mutex;
};
}