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

    auto write(std::string const &name, auto const &fun) -> void {
        auto const mapWriteMutex = std::lock_guard<std::shared_mutex>(this->mutex);
        fun(this->codices);
        if(this->waiters.contains(name))
            this->waiters.at(name).notify_all();
        if(this->codices.contains(name)) {
            if(this->codices.at(name).empty())
                this->codices.erase(name);
        }
    }

    auto read(auto const &fun) const -> void {
        auto const mapReadMutex = std::shared_lock<std::shared_mutex>(this->mutex);
        fun(this->codices);
    }

    auto writeWaitForExistance(std::string const &name, auto const &fun) -> void {
        auto const mapWriteMutex = std::lock_guard<std::shared_mutex>(this->mutex);
        while(!this->codices.contains(name))
            this->waiters[name].wait(this->mutex); // How do we remove a waiter?
        fun(this->codices);
        if(this->codices.contains(name)) {
            if(this->codices.at(name).empty())
                this->codices.erase(name);
        }
    }
    
    private:
        std::unordered_map<std::string, Codex> codices;
        std::unordered_map<std::string, std::condition_variable_any> waiters;
        mutable std::shared_mutex mutex;
};
}