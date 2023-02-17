#pragma once
#include "FSNG/PathSpaceTE.hpp"
#include "FSNG/utils.hpp"
#include "FSNG/Forge/Task.hpp"

#include <iostream>
#include <condition_variable>
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
    
    auto clearBlock(PathSpaceTE &space) -> void {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        this->currentlyDeleting.insert(&space);
        while(this->currentlyRunning.contains(&space))
            this->tasksChanged.wait(writeLock);
        this->currentlyDeleting.erase(&space);
    }
private:
    auto executor() -> void {
        while(this->isAlive) {
            if(std::optional<Task> taskOpt = this->grabNewTask()) {
                Task &task = *taskOpt;
                if(auto* fun = std::get_if<std::function<Coroutine()>>(&task.fun))
                    this->loop((*fun)(), task);
                else if(auto* fun = std::get_if<std::function<CoroutineVoid()>>(&task.fun))
                    this->loop((*fun)(), task);
                if(!this->currentlyDeleting.contains(task.space))
                    task.space->insert("", task.ticket, task.path); // remove coroutine
                auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
                this->currentlyRunning[task.space]--;
                if(this->currentlyRunning[task.space]==0)
                    this->currentlyRunning.erase(task.space);
                this->tasksChanged.notify_all();
            } else {
                return;
            }
        }
    }

    auto grabNewTask() -> std::optional<Task> {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        while(this->isAlive) {
            if(this->tasks.size()>0) {
                auto const task = this->tasks.begin()->second;
                this->tasks.erase(task.ticket);
                this->currentlyRunning[task.space]++;
                return task;
            }
            this->tasksChanged.wait(writeLock);
        }
        return std::nullopt;
    }

    auto loop(auto &&coroutine, auto &task) -> void {
        bool shouldGoAgain = false;
        do {
            shouldGoAgain = coroutine.next();
            bool shouldInsert = false;
            {
                auto readLock = std::shared_lock(this->mutex);
                shouldInsert = coroutine.hasValue() && !this->currentlyDeleting.contains(task.space);
            }
            if(shouldInsert)
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
    std::set<PathSpaceTE*> currentlyDeleting;
    std::map<PathSpaceTE*, int> currentlyRunning;
};

}