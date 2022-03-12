#pragma once
#include "FSNG/Codex.hpp"

#include <unordered_map>
#include <shared_mutex>

namespace FSNG {
struct CodicesAegis {
    CodicesAegis() = default;
    CodicesAegis(CodicesAegis const &other) : codices(other.codices) {}

    auto count(auto const &str) const -> int{
        return this->codices.count(str);
    }

    auto push_back(auto const &name, PathSpaceTE const &space) {
        this->codices[name].insert(space);
    }

    auto push_back(auto const &name, Data const &data) {
        this->codices[name].insert(data);
    }

    template<typename T>
    auto visitFirst(auto const &name, auto const &fun) {
        if(this->codices.count(name)>0) {
            return this->codices[name].template visitFirst<T>(fun);
        }
        return false;
    }

    auto readMutex() const -> std::shared_lock<std::shared_mutex> {
        return std::shared_lock<std::shared_mutex>(this->mutex);
    }

    auto writeMutex() const -> std::lock_guard<std::shared_mutex> {
        return std::lock_guard<std::shared_mutex>(this->mutex);
    }

    std::unordered_map<std::string, Codex> codices;
    mutable std::shared_mutex mutex;
};
}