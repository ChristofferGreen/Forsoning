#pragma once
#include <mutex>
#include <shared_mutex>

namespace FSNG {

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
    auto lock_shared() -> void { // unlocked -> shared
        this->shared.lock_shared();
    }

    auto unlock_shared() -> void { // shared -> unlocked
        this->shared.unlock_shared();
    }

    auto lock() -> void { // unlocked -> exclusive
        this->exclusive.lock();
        this->shared.lock();
    }

    auto unlock() -> void { // exclusive -> unlocked
        this->shared.unlock();
        this->exclusive.unlock();
    }

    auto lock_upgrade() -> void { // unlocked -> upgraded
        this->exclusive.lock();
        this->shared.lock_shared();
    }

    auto unlock_upgrade() -> void { // upgraded -> unlocked
        this->shared.unlock_shared();
        this->exclusive.unlock();
    }

    auto unlock_upgrade_and_lock() -> void { // upgraded -> exclusive
        this->shared.unlock_shared();
        this->shared.lock();
    }

    auto unlock_and_lock_upgrade() -> void { // exclusive -> upgraded
        this->shared.unlock();
        this->shared.lock_shared();
    }

    auto unlock_and_lock_shared() -> void { // exclusive -> shared
        this->shared.unlock();
        this->shared.lock_shared();
        this->exclusive.unlock();
    }

    auto unlock_upgrade_and_lock_shared() -> void { // upgraded -> shared
        this->exclusive.unlock();
    }
private:
    mutable std::shared_mutex shared;
    mutable std::mutex exclusive;
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