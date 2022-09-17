#pragma once
#include <mutex>
#include <shared_mutex>

#ifdef LOG_MUTEX
#define LOG_M(...) LOG("<TAG:Mutex>" __VA_ARGS__)
#define LogRAII_M(arg) LogRAII("<TAG:Mutex>"+arg)
#else
#define LOG_M(...)
#define LogRAII_M(...) 0
#endif

namespace FSNG {

template<typename T>
struct MutexLog {
    MutexLog() {}
    MutexLog(std::string const &name) : name(name) {}
    auto lock()          {LOG_M("Starting lock for {} {} current status {}",          this->name, fmt::ptr(this), this->status)this->mutex.lock();         this->status = 2;LOG_M("Lock finished, status for {} {} {}",          this->name, this->status, fmt::ptr(this))}
    auto unlock()        {LOG_M("Starting unlock for {} {} current status {}",        this->name, fmt::ptr(this), this->status)this->mutex.unlock();       this->status = 0;LOG_M("Unlock finished, status for {} {} {}",        this->name, this->status, fmt::ptr(this))}
    auto lock_shared()   {LOG_M("Starting lock_shared for {} {} current status {}",   this->name, fmt::ptr(this), this->status)this->mutex.lock_shared();  this->status = 1;LOG_M("Lock_shared finished, status for {} {} {}",   this->name, this->status, fmt::ptr(this))}
    auto unlock_shared() {LOG_M("Starting unlock_shared for {} {} current status {}", this->name, fmt::ptr(this), this->status)this->mutex.unlock_shared();this->status = 0;LOG_M("Unlock_shared finished, status for {} {} {}", this->name, this->status, fmt::ptr(this))}
private:
    T mutex;
    std::atomic<int> status = 0;
    std::string name;
};

/* 
https://codereview.stackexchange.com/questions/243177/implementing-boostupgrade-mutex-using-only-standard-locks
https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3427.html   
 
  exclusive ⬉
   ↑    │     \
   │    │      ⬊
   │    │       upgraded
   │    │      /   ↑
   │    ↓     ↙    │
   │  shared       │
   │    ↑          │
   ↓    ↓          │
  unlocked <───────╯

*/
struct UpgradableMutex {
    UpgradableMutex(std::string const &name) : name(name) {
        int a = 0;
        a++;
    }

    auto lock_shared() -> void { // unlocked -> shared
        auto raii = LogRAII_M(std::string("lock_shared ")+this->name);
        this->shared.lock_shared();
    }

    auto unlock_shared() -> void { // shared -> unlocked
        auto raii = LogRAII_M(std::string("unlock_shared ")+this->name);
        this->shared.unlock_shared();
    }

    auto lock() -> void { // unlocked -> exclusive
        auto raii = LogRAII_M(std::string("lock ")+this->name);
        this->exclusive.lock();
        this->shared.lock();
    }

    auto unlock() -> void { // exclusive -> unlocked
        auto raii = LogRAII_M(std::string("unlock ")+this->name);
        this->shared.unlock();
        this->exclusive.unlock();
    }

    auto lock_upgrade() -> void { // unlocked -> upgraded
        auto raii = LogRAII_M(std::string("lock_upgrade ")+this->name);
        this->exclusive.lock();
        this->shared.lock_shared();
    }

    auto unlock_upgrade() -> void { // upgraded -> unlocked
        auto raii = LogRAII_M(std::string("unlock_upgrade ")+this->name);
        this->shared.unlock_shared();
        this->exclusive.unlock();
    }

    auto unlock_upgrade_and_lock() -> void { // upgraded -> exclusive
        auto raii = LogRAII_M(std::string("unlock_upgrade_and_lock ")+this->name);
        this->shared.unlock_shared();
        this->shared.lock();
    }

    auto unlock_and_lock_upgrade() -> void { // exclusive -> upgraded
        auto raii = LogRAII_M(std::string("unlock_and_lock_upgrade ")+this->name);
        this->shared.unlock();
        this->shared.lock_shared();
    }

    auto unlock_and_lock_shared() -> void { // exclusive -> shared
        auto raii = LogRAII_M(std::string("unlock_and_lock_shared ")+this->name);
        this->shared.unlock();
        this->shared.lock_shared();
        this->exclusive.unlock();
    }

    auto unlock_upgrade_and_lock_shared() -> void { // upgraded -> shared
        auto raii = LogRAII_M(std::string("unlock_upgrade_and_lock_shared ")+this->name);
        this->exclusive.unlock();
    }
    std::string name;
private:
    mutable MutexLog<std::shared_mutex> shared    = MutexLog<std::shared_mutex>("shared");
    mutable MutexLog<std::mutex>        exclusive = MutexLog<std::mutex>       ("exclusive");
};

struct UnlockedToSharedLock {
    UnlockedToSharedLock(UpgradableMutex &mutex) : mutex(&mutex) {
        this->mutex->lock_shared();
    }
    ~UnlockedToSharedLock() {
        this->mutex->unlock_shared();
    }
    UpgradableMutex *mutex;
};

struct UnlockedToUpgradedLock {
    UnlockedToUpgradedLock(UpgradableMutex &mutex) : mutex(&mutex) {
        this->mutex->lock_upgrade();
    }
    ~UnlockedToUpgradedLock() {
        this->mutex->unlock_upgrade();
    }
    UpgradableMutex *mutex;
};

struct UpgradedToExclusiveLock {
    UpgradedToExclusiveLock(UpgradableMutex &mutex) : mutex(&mutex) {
        this->mutex->unlock_upgrade_and_lock();
    }
    ~UpgradedToExclusiveLock() {
        this->mutex->unlock_and_lock_upgrade();
    }
    UpgradableMutex *mutex;
};

struct UnlockedToExclusiveLock {
    UnlockedToExclusiveLock(UpgradableMutex &mutex) : mutex(&mutex) {
        this->mutex->lock();
    }
    ~UnlockedToExclusiveLock() {
        this->mutex->unlock();
    }
    UpgradableMutex *mutex;
};

struct UpgradableMutexWaitableWrapper {
    UpgradableMutexWaitableWrapper(UpgradableMutex &mutex) : mutex(&mutex) {}
    auto lock() -> void {
        this->mutex->lock_upgrade();
    }
    auto unlock() -> void {
        this->mutex->unlock_upgrade();
    }
    UpgradableMutex *mutex;
};

}