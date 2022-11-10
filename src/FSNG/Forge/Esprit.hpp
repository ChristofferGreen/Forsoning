#pragma once
#include "FSNG/Forge/Ticket.hpp"
#include "FSNG/Forge/LoggableMutex.hpp"

#include <set>
#include <mutex>
#include <condition_variable>

namespace FSNG {
struct Esprit {
    Esprit() : mutex("Esprit") {}

    auto activate(Ticket const &ticket, PathSpaceTE &space) -> void {
        auto writeLock = std::unique_lock<LoggableMutex<std::shared_mutex>>(this->mutex);
        this->active[&space].insert(ticket);
    }

    auto deactivate(Ticket const &ticket, PathSpaceTE &space) -> void {
        auto writeLock = std::unique_lock<LoggableMutex<std::shared_mutex>>(this->mutex);
        this->active[&space].erase(ticket);
        if(this->active.count(&space))
            if(this->active.at(&space).size()==0)
                this->active.erase(&space);
        this->condition.notify_all();
    }

    auto wait(PathSpaceTE &space) const -> void {
        auto readLock = std::shared_lock(this->mutex);
        while(this->active.count(&space))
            this->condition.wait(readLock);
    }

    auto wait(Ticket const &ticket) const -> void {
        auto readLock = std::shared_lock(this->mutex);
        bool shouldTryAgain = false;
        do {
            shouldTryAgain = false;
            for(auto const &a : this->active) {
                if(a.second.contains(ticket)) {
                    this->condition.wait(readLock);
                    shouldTryAgain = true;
                    break;
                }
            }
        } while(shouldTryAgain);
    }

    auto nbrActive() const -> int {
        auto readLock = std::shared_lock(this->mutex);
        int nbr = 0;
        for(auto const &a : this->active)
            nbr += a.second.size();
        return nbr;
    }

private:
    std::map<PathSpaceTE*, std::set<Ticket>> active;
    mutable LoggableMutex<std::shared_mutex> mutex;
    mutable std::condition_variable_any condition;
};
}