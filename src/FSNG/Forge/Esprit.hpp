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
        this->activated++;
        this->active[&space].insert(ticket);
    }

    auto startedRunning(Ticket const &ticket, PathSpaceTE &space) -> void {
        auto writeLock = std::unique_lock<LoggableMutex<std::shared_mutex>>(this->mutex);
        this->runningCounter++;
        this->running[&space].insert(ticket);
    }

    auto stoppedRunning(Ticket const &ticket, PathSpaceTE &space) -> void {
        auto writeLock = std::unique_lock<LoggableMutex<std::shared_mutex>>(this->mutex);
        this->stoppedRunningCounter++;
        this->running[&space].erase(ticket);
        if(this->running[&space].size()==0)
            this->running.erase(&space);
        this->condition.notify_all();
    }

    auto deactivate(PathSpaceTE &space) -> void {
        auto writeLock = std::unique_lock<LoggableMutex<std::shared_mutex>>(this->mutex);
        if(this->active.count(&space)) {
            for(auto const &space : this->active.at(&space))
                this->deactivated++;
            this->active.erase(&space);
        }
        this->condition.notify_all();
    }

    auto deactivate(Ticket const &ticket, PathSpaceTE &space) -> void {
        auto writeLock = std::unique_lock<LoggableMutex<std::shared_mutex>>(this->mutex);
        this->deactivated++;
        if(this->active.contains(&space))
            this->active.at(&space).erase(ticket);
        if(this->active.count(&space))
            if(this->active.at(&space).size()==0)
                this->active.erase(&space);
        this->condition.notify_all();
    }

    auto wait(PathSpaceTE &space) const -> void {
        auto readLock = std::shared_lock(this->mutex);
        while(this->active.count(&space) || this->running.count(&space))
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
    std::map<PathSpaceTE*, std::set<Ticket>> running;
    mutable LoggableMutex<std::shared_mutex> mutex;
    mutable std::condition_variable_any condition;
    mutable int deactivated = 0;
    mutable int activated = 0;
    mutable int runningCounter = 0;
    mutable int stoppedRunningCounter = 0;
};
}