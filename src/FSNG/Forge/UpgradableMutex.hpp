#pragma once
#include <mutex>
#include <shared_mutex>

namespace FSNG {

struct UpgradableMutex {
    auto lock_shared() -> void {
        this->shared.lock_shared();
        this->state = 1;
    }

    auto upgrade() -> void {
        this->upgrading.lock(); // Now the only upgrading thread
        this->shared.unlock_shared();
        this->shared.lock(); // Now the only shared mode and upgrading thread
        this->unique.lock(); // Now we have write rights
        this->state = 2;
        this->upgraded = true;
    }

    auto downgrade() -> void {
        this->unique.unlock();
        this->upgrading.unlock();
        this->shared.unlock();
        this->shared.lock_shared();
        this->state = 1;
        this->upgraded = false;
    }

    auto unlock_shared() -> void {
        this->shared.unlock_shared();
        this->state = 0;
    }

private:
    mutable std::shared_mutex shared;
    mutable std::mutex upgrading;
    mutable std::mutex unique;
    int state = 0;
    bool upgraded = false;
};

struct Upgrade {
    Upgrade(std::shared_lock<UpgradableMutex> &lock) {
        this->lock = lock.mutex();
        this->lock->upgrade();
    }
    ~Upgrade() {
        this->lock->downgrade();
    }
    UpgradableMutex *lock;
};

}