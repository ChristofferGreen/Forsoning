#pragma once
#include "FSNG/Forge/Task.hpp"

#include <iostream>

#include "spdlog/spdlog.h"

namespace FSNG {
struct Eschelon {
    auto add(Ticket const &ticket, std::function<Coroutine()> const &coroutineFun, std::function<void(Data const &data)> const &inserter) -> void {
        auto const writeLock = std::lock_guard<std::shared_mutex>(this->mutex);
        this->tasks[ticket] = Task{ticket, coroutineFun, inserter};
        this->condition.notify_all();
        LOG("Added task to eschelon with ticket: {}, total tasks: {},  waiters: {}", ticket, this->tasks.size(), this->waiters);
    }

    auto remove(Ticket const &ticket) -> bool {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        return this->tasks.erase(ticket)>0;
    }

    auto popWait() -> std::optional<Task> {
        LOG("Eschelon::popWait enter");
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        while(this->isAlive && this->tasks.size()==0) {
            this->waiters++;
            this->condition.wait(writeLock);
            this->waiters--;
        }
        if(!this->isAlive)
            return std::nullopt;
        auto const task = *this->tasks.begin();
        this->tasks.erase(this->tasks.begin());
        this->condition.notify_all();
        LOG("Eschelon::popWait exit");
        return task.second;
    }

    auto wait(Ticket const &ticket) -> void {
        LOG("Eschelon::wait grabbing read mutex for ticket {}", ticket);
        auto readLock = std::shared_lock(this->mutex);
        LOG("Eschelon::wait grabbed read mutex for ticket {}, tickets {}, contains ticket {}", ticket, this->tasks.size(), this->tasks.contains(ticket));
        for(auto const &task : this->tasks)
            LOG("Eschelon::wait has task ticket {}", task.first);
        while(this->tasks.contains(ticket))
            this->condition.wait(readLock);
        LOG("Eschelon::wait (woke up) exiting for ticket {}", ticket);
    }

    auto shutdown() -> void {
        this->isAlive = false;
        while(this->waiters>0) {this->condition.notify_all();}
    }

    auto size() const -> int {
        auto readLock = std::shared_lock(this->mutex);
        return this->tasks.size();
    }

    auto newTicket() -> Ticket {
        return this->currentTicket++;
    }

private:
    std::map<Ticket, Task> tasks;
    std::atomic<bool> isAlive = true;
    std::atomic<int> waiters = 0;
    Ticket currentTicket = FirstTicket;
    mutable std::shared_mutex mutex;
    mutable std::condition_variable_any condition;
};
}