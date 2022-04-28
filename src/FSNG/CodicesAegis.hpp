#pragma once
#include "FSNG/Codex.hpp"

#include <unordered_map>
#include <shared_mutex>

namespace FSNG {
struct CodicesAegis {
    CodicesAegis() = default;
    CodicesAegis(CodicesAegis const &other) : codices(other.codices) {}

    template<typename T>
    auto visitFirst(auto const &name, auto const &fun) {
        if(this->codices.contains(name)) {
            return this->codices[name].template visitFirst<T>(fun);
        }
        return false;
    }

    auto writeCodices(auto const &fun) -> void {
        auto const mapWriteMutex = this->writeMutex();
        fun(this->codices);
    }

    auto readCodices(auto const &fun) const -> void {
        auto const mapReadMutex = this->readMutex();
        fun(this->codices);
    }
    private:
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