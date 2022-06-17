#pragma once
#include "FSNG/Forge/Task.hpp"

#include <iostream>

#define FMT_HEADER_ONLY
#include "fmt/format.h"

namespace FSNG {
struct Eschelon {
    auto add(std::function<Coroutine()> const &coroutineFun, std::function<void(Data const &data)> const &inserter) -> Ticket {
        auto const writeLock = std::lock_guard<std::shared_mutex>(this->mutex);
        auto const ticket = this->currentTicket++;
        this->tasks[ticket] = Task{ticket, coroutineFun, inserter};
        this->condition.notify_all();
        printm(fmt::format("Added task to eschelon with ticket: {}, total tasks: {},  waiters: {}", ticket, this->tasks.size(), this->waiters));
        return ticket;
    }

    auto remove(Ticket const &ticket) -> bool {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        return this->tasks.erase(ticket)>0;
    }

    auto popWait() -> std::optional<Task> {
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
        return task.second;
    }

    auto wait(Ticket const &ticket) -> void {
        auto readLock = std::shared_lock(this->mutex);
        while(this->tasks.contains(ticket))
            this->condition.wait(readLock);
    }

    auto shutdown() -> void {
        this->isAlive = false;
        this->condition.notify_all();
        while(this->waiters>0) {}
    }

    auto size() const -> int {
        auto readLock = std::shared_lock(this->mutex);
        return this->tasks.size();
    }
private:
    std::map<Ticket, Task> tasks;
    bool isAlive = true;
    std::atomic<int> waiters = 0;
    Ticket currentTicket = FirstTicket;
    mutable std::shared_mutex mutex;
    mutable std::condition_variable_any condition;
};
}