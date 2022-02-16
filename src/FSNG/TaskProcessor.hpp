#pragma once
#include "FSNG/Task.hpp"

#include <atomic>
#include <deque>
#include <shared_mutex>
#include <condition_variable>
#include <thread>
#include <vector>

namespace FSNG {

struct TaskProcessor {
    TaskProcessor() {
        for(auto i = 0; i < std::thread::hardware_concurrency(); ++i)
            threads.emplace_back(&TaskProcessor::executor, this);
    }
    ~TaskProcessor() {
        this->alive = false;
        this->condition.notify_all();
        for(auto &thread : this->threads)
            thread.join();
    }

    auto add(void *id, std::function<Coroutine()> const &coroutineFun, std::function<void(Data const &data)> const &inserter) {
        auto const writeLock = std::lock_guard<std::shared_mutex>(this->mutex);
        this->tasks.push_back(Task{id, coroutineFun, inserter});
        this->condition.notify_one();
    }

    auto executor() -> void {
        while(this->alive) {
            if(auto task = popTaskWait()) {
                auto coroutine = task.value().fun();
                while(coroutine.next())
                    task.value().inserter(coroutine.getValue());
            }
        }
    }

private:
    auto popTaskWait() -> std::optional<Task> {
        while(this->alive) {
            auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
            if(this->tasks.size()>0) {
                auto task = this->tasks.front();
                this->tasks.pop_front();
                return task;
            }
            this->condition.wait(writeLock);
        }
        return std::nullopt;
    }
    std::atomic<bool> alive = true;
    std::deque<Task> tasks;
    std::vector<std::thread> threads;
    mutable std::shared_mutex mutex;
    mutable std::condition_variable_any condition;
};
}