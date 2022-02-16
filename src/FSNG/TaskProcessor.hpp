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
            threads.emplace_back(std::thread(&TaskProcessor::executor, this));
    }
    ~TaskProcessor() {
        this->alive = false;
        for(auto &thread : this->threads)
            thread.join();
    }

    auto add(void *id, std::function<Coroutine()> const &coroutineFun, std::function<void(Data const &data)> const &inserter) {
        auto const writeLock = std::lock_guard<std::shared_mutex>(this->mutex);
        this->tasks.push_back(Task{id, coroutineFun, inserter});
    }

    auto executor() -> void {
        while(this->alive) {
            std::optional<Task> task;
            {
                auto const writeLock = std::lock_guard<std::shared_mutex>(this->mutex);
                if(this->tasks.size()>0) {
                    task = this->tasks.front();
                    this->tasks.pop_front();
                }
            }
            if(task) {
                auto coroutine = task.value().fun();
                while(coroutine.next())
                    task.value().inserter(coroutine.getValue());
            }
        }
    }
private:
    std::atomic<bool> alive = true;
    std::deque<Task> tasks;
    std::vector<std::thread> threads;
    mutable std::shared_mutex mutex;
    mutable std::condition_variable_any condition;
};
}