#pragma once
#include "FSNG/Forge/Task.hpp"
#include "FSNG/Forge/LoggableMutex.hpp"

#include <iostream>

#include "spdlog/spdlog.h"

#ifdef LOG_FORGE
#define LOG_E(...) LOG("<TAG:Echelon>" __VA_ARGS__)
#define LogRAII_E(...) LogRAII("<TAG:Echelon>" __VA_ARGS__)
#else
#define LOG_E(...)
#define LogRAII_E(...) 0
#endif

namespace FSNG {
struct Eschelon {
    Eschelon() : mutex("Echelon") {}
    
    auto add(Ticket const &ticket, auto const &coroutineFun, std::function<void(Data const &data, Ticket const &ticket, PathSpaceTE &space)> const &inserter, PathSpaceTE &space, Path const &path) -> void {
        auto const writeLock = std::lock_guard<LoggableMutex<std::shared_mutex>>(this->mutex);
        this->tasks[ticket] = Task(ticket, coroutineFun, inserter, &space, path);
        this->condition.notify_one();
        LOG_E("Added task to eschelon with ticket: {}, total tasks: {},  waiters: {}", ticket, this->tasks.size(), this->waiters.load());
    }

    auto remove(Ticket const &ticket) -> bool {
        auto writeLock = std::unique_lock<LoggableMutex<std::shared_mutex>>(this->mutex);
        return this->tasks.erase(ticket)>0;
    }

    auto remove(PathSpaceTE &space) {
        auto writeLock = std::unique_lock<LoggableMutex<std::shared_mutex>>(this->mutex);
        std::vector<Ticket> toBeRemoved;
        for(auto const &task : this->tasks)
            if(task.second.space == &space)
                toBeRemoved.push_back(task.first);
        for(auto const &ticket : toBeRemoved)
            this->tasks.erase(ticket);
    }

    auto popWait() -> std::optional<Task> {
        auto const raii = LogRAII_E("popWait");
        auto writeLock = std::unique_lock<LoggableMutex<std::shared_mutex>>(this->mutex);
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
    mutable LoggableMutex<std::shared_mutex> mutex;
    mutable std::condition_variable_any condition;
};
}