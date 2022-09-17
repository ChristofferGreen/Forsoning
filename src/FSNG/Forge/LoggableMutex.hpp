#pragma once

#include "FSNG/utils.hpp"

#ifdef LOG_LMUTEX
#define LOG_LM(...) LOG(std::string("<TAG:lm_")+this->name+">" __VA_ARGS__)
#define LogRAII_LM(...) LogRAII("<TAG:Echelon>" __VA_ARGS__)
#else
#define LOG_LM(...)
#define LogRAII_LM(...) 0
#endif

template<typename T>
struct LoggableMutex {
    LoggableMutex(std::string const &name) : name(name) {}
    auto lock() -> void {
        LOG_LM("lock start")
        this->mutex.lock();
        LOG_LM("lock finished")
    }
    auto unlock() -> void {
        LOG_LM("unlock start")
        this->mutex.unlock();
        LOG_LM("unlock finished")
    }
    auto lock_shared() -> void {
        LOG_LM("lock_shared start")
        this->mutex.lock_shared();
        LOG_LM("lock_shared finished")
    }
    auto unlock_shared() -> void {
        LOG_LM("unlock_shared start")
        this->mutex.unlock_shared();
        LOG_LM("unlock_shared finished")
    }
    T mutex;
    std::string name;
};