#pragma once
#include <mutex>
#include <shared_mutex>

namespace FSNG {

struct UpgradableMutex {
    auto lock_shared() -> void {
        this->shared.lock();
    }

    auto upgrade() -> void {
        this->upgraded.lock();
        this->shared.unlock();
        this->unique.lock();
    }

    auto downgrade() -> void {
        this->unique.unlock();
        this->shared.lock();
        this->upgraded.unlock();
    }

    auto unlock_shared() -> void {
        this->shared.unlock();
    }

private:
    mutable std::shared_mutex shared;
    mutable std::mutex upgraded;
    mutable std::mutex unique;

};

}