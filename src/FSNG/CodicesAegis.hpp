#pragma once
#include "FSNG/Codex.hpp"

#include <future>
#include <unordered_map>
#include <shared_mutex>

namespace FSNG {
struct CodicesAegis {
    CodicesAegis() = default;
    CodicesAegis(CodicesAegis const &other) : codices(other.codices) {}
    auto operator==(CodicesAegis const &rhs) const -> bool {
        auto const mapReadMutex = std::shared_lock<std::shared_mutex>(this->mutex);
        return this->codices==rhs.codices; 
    }

    auto write(std::string const &name, auto const &fun) {
        auto const mapWriteMutex = std::unique_lock<std::shared_mutex>(this->mutex);
        fun(this->codices);
        this->popEmptySpace(name);
    }

    auto writeRet(std::string const &name, auto const &fun) {
        auto const mapWriteMutex = std::unique_lock<std::shared_mutex>(this->mutex);
        auto const val = fun(this->codices);
        this->popEmptySpace(name);
        return val;
    }

    auto writeInsert(std::string const &name, auto const &fun) -> void {
        this->write(name, fun);
        this->condition.notify_all();
    }

    auto writeUntilSucess(std::string const &name, auto const &fun) -> void {
        auto mapWriteMutex = std::unique_lock<std::shared_mutex>(this->mutex);
        while(!fun(this->codices))
            this->condition.wait(mapWriteMutex);
        this->popEmptySpace(name);
    }

    auto read(auto const &fun) const -> void {
        auto const mapReadMutex = std::shared_lock<std::shared_mutex>(this->mutex);
        fun(this->codices);
    }

    auto waitForWrite() const -> void {
        auto readLock = std::shared_lock(this->mutex);
        this->condition.wait(readLock);
    }

    auto size() const -> int {
         auto readLock = std::shared_lock(this->mutex);
         return this->codices.size();
    }
    
    private:
        auto popEmptySpace(std::string const &name) -> void {
            if(this->codices.contains(name)) {
                if(this->codices.at(name).empty())
                    this->codices.erase(name);
            }
        }

        std::unordered_map<std::string, Codex> codices;
        mutable std::shared_mutex mutex;
        mutable std::condition_variable_any condition;
};
}