#pragma once
#include "FSNG/Forge/Ticket.hpp"

#include <set>
#include <mutex>
#include <condition_variable>

namespace FSNG {
struct Esprit {
    auto activate(Ticket const &ticket) {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        this->active.insert(ticket);
    }

    auto deactivate(Ticket const &ticket) {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        this->active.erase(ticket);
        this->deactivated.insert(ticket);
        this->condition.notify_all();
    }

    auto isActive(Ticket const &ticket) const -> bool {
        auto readLock = std::shared_lock(this->mutex);
        return this->active.contains(ticket);
    }

    auto wait(Ticket const &ticket) const -> void {
        auto readLock = std::shared_lock(this->mutex);
        while(this->active.contains(ticket))
            this->condition.wait(readLock);
    }

private:
    std::set<Ticket> active;
    std::set<Ticket> deactivated;
    mutable std::shared_mutex mutex;
    mutable std::condition_variable_any condition;
};
}