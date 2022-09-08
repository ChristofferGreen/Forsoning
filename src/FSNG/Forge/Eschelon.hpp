#pragma once
#include "FSNG/Forge/Task.hpp"

#include <iostream>

#include "spdlog/spdlog.h"

#ifdef LOG_FORGE
#define LOG_E(...) LOG("<TAG:Echelon>" __VA_ARGS__)
#define LogRAII_E(...) LogRAII("<TAG:Echelon>" __VA_ARGS__)
#else
#define LOG_F(...)
#define LogRAII_F(...) 0
#endif

namespace FSNG {
struct Eschelon {
    auto add(Ticket const &ticket, std::function<Coroutine()> const &coroutineFun, std::function<void(Data const &data, Ticket const &ticket)> const &inserter) -> void {
        auto const writeLock = std::lock_guard<std::shared_mutex>(this->mutex);
        this->tasks[ticket] = Task{ticket, coroutineFun, inserter};
        this->condition.notify_one();
        LOG_E("Added task to eschelon with ticket: {}, total tasks: {},  waiters: {}", ticket, this->tasks.size(), this->waiters.load());
    }

    auto remove(Ticket const &ticket) -> bool {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        return this->tasks.erase(ticket)>0;
    }

    auto popWait() -> std::optional<Task> {
        auto const raii = LogRAII_E("popWait");
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
        auto const raii = LogRAII_E("wait");
        auto readLock = std::shared_lock(this->mutex);
        while(this->tasks.contains(ticket))
            this->condition.wait(readLock);
    }

    auto shutdown() -> void {
        auto const raii = LogRAII_E("shutdown");
        this->isAlive = false;
        while(this->waiters>0) {this->condition.notify_all();}
    }

    auto size() const -> int {
        auto readLock = std::shared_lock(this->mutex);
        return this->tasks.size();
    }

    auto newTicket() -> Ticket {
        auto const raii = LogRAII_E("newTicket");
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