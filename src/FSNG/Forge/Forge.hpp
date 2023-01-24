#pragma once
#include "FSNG/PathSpaceTE.hpp"
#include "FSNG/utils.hpp"
#include "FSNG/Forge/Task.hpp"

#include "spdlog/spdlog.h"

#include <iostream>

#ifdef LOG_FORGE
#define LOG_F(...) LOG("<TAG:Forge>" __VA_ARGS__)
#define LogRAII_F(...) LogRAII("<TAG:Echelon>" __VA_ARGS__)
#else
#define LOG_F(...)
#define LogRAII_F(...) 0
#endif

namespace FSNG {
struct Forge {
    auto static CreateSingleton() -> void {
        Forge::instance_ = std::make_unique<Forge>();
    }
    auto static DestroySingleton() -> void {
        Forge::instance_.release();
    }

    auto static instance() -> std::unique_ptr<Forge>& {
        return Forge::instance_;
    }

    ~Forge() {
        {
            auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
            this->isAlive = false;
            this->tasksChanged.notify_all();
            while(this->tasks.size())
                this->tasksChanged.wait(writeLock);
        }
        for(auto &thread : this->threads)
            thread.join();
    }

    auto add(auto const &coroutineFun, PathSpaceTE &space, Path const &path, Path const &coroResultPath="") -> Ticket {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        auto const ticket = this->newTicket();
        this->tasks[ticket] = Task(ticket, coroutineFun, &space, path, coroResultPath);
        auto const hasFreeThreads = this->threads.size()>this->tasks.size();
        auto const lessAllocatedThreadsThanMaximum = this->threads.size()<(std::thread::hardware_concurrency()*2);
        if(!hasFreeThreads && lessAllocatedThreadsThanMaximum)
            this->threads.emplace_back(std::thread(&Forge::executor, this));
        this->tasksChanged.notify_all();
        return ticket;
    }

    auto wait(Ticket const &ticket) const -> void {
        auto readLock = std::shared_lock(this->mutex);
        while(this->tasks.contains(ticket))
            this->tasksChanged.wait(readLock);
    }

    auto remove(Ticket const &ticket) -> void {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        this->tasks.erase(ticket);
        this->tasksChanged.notify_all();
    }
    
    auto clearBlock(PathSpaceTE &space) -> void {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        std::vector<Ticket> running;
        std::vector<Ticket> toDelete;
        for(auto it = this->tasks.cbegin(); it != this->tasks.cend(); ++it) {
            if(it->second.space==&space) {
                if(it->second.isRunning)
                    running.push_back(it->first);
                else
                    toDelete.push_back(it->first);
                    
            }
        }
        for(auto const &ticket : toDelete)
            this->tasks.erase(ticket);
        for(auto const &ticket : running)
            while(this->tasks.contains(ticket))
                this->tasksChanged.wait(writeLock);
    }
private:
    auto executor() -> void {
        while(this->isAlive) {
            if(std::optional<Ticket> ticket = this->launchNewTask()) {
                auto &task = this->tasks.at(ticket.value());
                if(auto* fun = std::get_if<std::function<Coroutine()>>(&task.fun))
                    this->loop((*fun)(), task);
                else if(auto* fun = std::get_if<std::function<CoroutineVoid()>>(&task.fun))
                    this->loop((*fun)(), task);
                task.space->grab<void>(task.path, ticket.value());
                auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
                this->tasks.erase(ticket.value());
                this->tasksChanged.notify_all();
            } else {
                return;
            }
        }
    }

    auto launchNewTask() -> std::optional<Ticket> {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        while(this->isAlive) {
            for(auto &p : this->tasks) {
                if(!p.second.isRunning) {
                    p.second.isRunning=true;
                    return p.first;
                }
            }
            this->tasksChanged.wait(writeLock);
        }
        return std::nullopt;
    }

    auto loop(auto &&coroutine, auto &task) -> void {
        bool shouldGoAgain = false;
        do {
            shouldGoAgain = coroutine.next();
            if(coroutine.hasValue())
                //task.inserter(coroutine.getValue(), task.ticket, *task.space);
                task.space->insert(task.coroResultPath!="" ? task.coroResultPath : task.path, coroutine.getValue());
        } while(shouldGoAgain && this->isAlive);
    }

    auto newTicket() -> Ticket {
        return this->currentTicket++;
    }

    inline static std::unique_ptr<Forge> instance_;
    std::atomic<bool> isAlive = true;
    mutable std::shared_mutex mutex;
    mutable std::condition_variable_any tasksChanged;
    std::vector<std::thread> threads;
    std::map<Ticket, Task> tasks;
    Ticket currentTicket = FirstTicket;
};

}