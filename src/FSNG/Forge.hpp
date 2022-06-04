#pragma once
#include "FSNG/Task.hpp"
#include "FSNG/Thread.hpp"
#include "FSNG/Ticket.hpp"

#include <atomic>
#include <deque>
#include <shared_mutex>
#include <condition_variable>
#include <thread>
#include <vector>

namespace FSNG {
struct Forge {
    Forge() {
        for(auto i = 0; i < std::thread::hardware_concurrency()*2; ++i)
            this->threads.push_back(Thread{false, 0, std::thread(&Forge::executor, this)});
        this->availableThreads = this->threads.size();
    }

    ~Forge() {
        this->alive = false;
        this->condition.notify_all();
        for(auto &thread : this->threads)
            thread.thread.join();
    }

    auto add(std::function<Coroutine()> const &coroutineFun, std::function<void(Data const &data)> const &inserter) -> Ticket {
        auto const writeLock = std::lock_guard<std::shared_mutex>(this->mutex);
        auto const ticket = this->currentTicket;
        this->currentTicket++;
        this->tasks[ticket] = Task{coroutineFun, inserter};
        if(this->availableThreads==0) {
            this->threads.push_back(Thread{true, ticket, std::thread(&Forge::executor, this)});
            this->availableThreads++;
        }
        this->condition.notify_one();
        return ticket;
    }

    auto remove(Ticket const &ticket) -> void {
        auto const writeLock = std::lock_guard<std::shared_mutex>(this->mutex);
        this->tasks.erase(ticket);
    }

    auto executor() -> void {
        while(this->alive) {
            if(auto task = popTaskWait()) {
                this->availableThreads--;
                auto coroutine = task.value().fun();
                while(coroutine.next())
                    task.value().inserter(coroutine.getValue());
                this->availableThreads++;
            }
        }
    }

private:
    auto popTaskWait() -> std::optional<Task> {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        while(this->alive && this->tasks.size()==0)
            this->condition.wait(writeLock);
        if(!this->alive)
            return std::nullopt;
        auto task = *this->tasks.begin();
        this->tasks.erase(this->tasks.begin());
        return task.second;
    }

    std::atomic<bool> alive = true;
    std::atomic<int> availableThreads;
    std::map<Ticket, Task> tasks;
    std::vector<Thread> threads;
    Ticket currentTicket = 0;
    mutable std::shared_mutex mutex;
    mutable std::condition_variable_any condition;
};
}