#pragma once
#include "FSNG/Coroutine.hpp"
#include "FSNG/Codex.hpp"
#include "FSNG/Data.hpp"
#include "FSNG/Path.hpp"
#include "FSNG/PathSpaceTE.hpp"
#include "FSNG/Security.hpp"
#include "FSNG/Forge/UpgradableMutex.hpp"
#include "FSNG/utils.hpp"

#include "nlohmann/json.hpp"

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

#ifdef LOG_PATH_SPACE
#define LOG_PS(...) LOG("<TAG:PathSpace>" __VA_ARGS__)
#define LogRAII_PS(...) LogRAII("<TAG:PathSpace>" __VA_ARGS__)
#else
#define LOG_PS(...)
#define LogRAII_PS(...) 0
#endif

namespace FSNG {
struct PathSpace {
    PathSpace(std::string const &name="/") : mutex(name) {
        LOG_PS("Creating PathSpace with name {}", name);
    };
    PathSpace(PathSpace const &rhs) : mutex(rhs.mutex.name) {
        LOG_PS("Creating copy of PathSpace with name {}", rhs.mutex.name);
        UnlockedToExclusiveLock lock(this->mutex);
        UnlockedToSharedLock lockRHS(rhs.mutex);
        this->codices = rhs.codices;
    }
    PathSpace(PathSpace const &rhs, PathSpaceTE *root) : mutex(rhs.mutex.name), root(root) {
        LOG_PS("Creating copy of PathSpace with name {}", rhs.mutex.name);
        UnlockedToExclusiveLock lock(this->mutex);
        UnlockedToSharedLock lockRHS(rhs.mutex);
        this->codices = rhs.codices;
    }
    auto preDestruct() -> void {
        Forge::instance()->clearBlock(*this->root);
    }

    auto operator==(PathSpace const &rhs) const -> bool { return this->codices==rhs.codices; }

    auto setRoot(PathSpaceTE *root) {
        this->root = root;
    }

    auto removeCoroutine(Path const &range, Ticket const &ticket) -> bool {
        if(range.isAtData()) {
            UnlockedToExclusiveLock upgraded(this->mutex);
            if(this->codices.contains(range.dataName())) {
                this->codices[range.dataName()].removeCoroutine(ticket);
                if(this->codices[range.dataName()].empty())
                    this->codices.erase(range.dataName());
                this->condition.notify_all();
                return true;
            }
        } else {
            UnlockedToUpgradedLock lock(this->mutex);
            auto const spaceName = range.spaceName().value();
            if(this->codices.contains(spaceName)) {
                UpgradedToExclusiveLock upgraded(this->mutex);
                return codices[spaceName].template visitFirst<PathSpaceTE>([&range, ticket](auto &space){return space.template grab<Coroutine>(range.next(), ticket);});
            }
        }
        return false;
    }

    auto grab(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        auto const raii = LogRAII_PS("grab "+range.dataName());
        return range.isAtData() ? this->grabDataName(range.dataName(), info, data, isTriviallyCopyable) :
                                  this->grabSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable);
    }

    auto grabBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        auto const raii = LogRAII_PS("grabBlock "+range.dataName());
        bool const shouldWait = true;
        if(range.isAtRoot()) {
            bool found = false;
            while(!found)
                found = range.isAtData() ? this->grabDataName(range.dataName(), info, data, isTriviallyCopyable, shouldWait) :
                                           this->grabSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable, shouldWait);
            return found;
        } else
            return range.isAtData() ? this->grabDataName(range.dataName(), info, data, isTriviallyCopyable) :
                                      this->grabSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable, shouldWait);
    }

    auto read(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        auto const raii = LogRAII_PS("read "+range.dataName());
        UnlockedToSharedLock lock(this->mutex);
        return range.isAtData() ?  this->readDataName(range.dataName(), info, data, isTriviallyCopyable) :
                                   this->readSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable);
    }

    auto readBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        auto const raii = LogRAII_PS("readBlock "+range.dataName());
        if(range.isAtRoot()) {
            bool found = false;
            while(!found) {
                UnlockedToSharedLock lock(this->mutex);
                bool const shouldWait = true;
                found = range.isAtData() ? this->readDataName(range.dataName(), info, data, isTriviallyCopyable, shouldWait) :
                                           this->readSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable, shouldWait);
            }
            return found;
        } else {
            UnlockedToSharedLock lock(this->mutex);
            return range.isAtData() ? this->readDataName(range.dataName(), info, data, isTriviallyCopyable) :
                                      this->readSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable);
        }
    }

    virtual auto insert(Path const &range, Data const &data, Path const &coroResultPath="") -> bool {
        auto const raii = LogRAII_PS("insert "+range.dataName());
        return range.isAtData() ? this->insertDataName(range, range.dataName(), data, coroResultPath) :
                                  this->insertSpaceName(range, range.spaceName().value(), data, coroResultPath);
    }

    virtual auto toJSON() const -> nlohmann::json {
        auto const raii = LogRAII_PS("toJSON");
        nlohmann::json json;
        UnlockedToSharedLock lock(this->mutex);
        for(auto const &p : codices)
            json[p.first] = p.second.toJSON();
        return json;
    }

    auto hasListeners() const -> bool {
        return this->listeners>0;
    }

private:
    virtual auto grabDataName(std::string const &dataName, std::type_info const *info, void *data, bool isTriviallyCopyable, bool const shouldWait = false) -> bool {
        auto const raii = LogRAII_PS("grabDataName "+dataName);
        LOG_PS("grabDataName Starting UnlockedToUpgradedLock");
        UnlockedToUpgradedLock lock(this->mutex);
        LOG_PS("grabDataName Finished UnlockedToUpgradedLock");
        bool found = false;
        if(this->codices.contains(dataName)) {
            LOG_PS("grabDataName Starting UpgradedToExclusiveLock");
            UpgradedToExclusiveLock upgradedLock(this->mutex);
            LOG_PS("grabDataName Finished UpgradedToExclusiveLock");
            found = codices.at(dataName).grab(info, data, isTriviallyCopyable);
            LOG_PS("grabDataName find result {}", found);
            if(!found && shouldWait) {
                auto u = UpgradableMutexWaitableWrapper(this->mutex);
                this->listeners++;
                this->condition.wait(u);
                this->listeners--;
            }
        }
        return found;
    }

    virtual auto grabSpaceName(std::string const &spaceName, Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable, bool const shouldWait = false) -> bool {
        auto const raii = LogRAII_PS("grabSpaceName "+spaceName);
        UnlockedToUpgradedLock lock(this->mutex);
        if(this->codices.contains(spaceName)) {
            bool const found = codices[spaceName].template visitFirst<PathSpaceTE>([&range, info, data, isTriviallyCopyable, shouldWait](auto &space){
                if(shouldWait)
                    return space.grabBlock(range.next(), info, data, isTriviallyCopyable);
                else
                    return space.grab(range.next(), info, data, isTriviallyCopyable);
            });
            if(!found && shouldWait) {
                auto u = UpgradableMutexWaitableWrapper(this->mutex);
                this->listeners++;
                this->condition.wait(u);
                this->listeners--;
            }
            return found;
        }
        return false;
    }

    virtual auto readDataName(std::string const &dataName, std::type_info const *info, void *data, bool isTriviallyCopyable, bool const shouldWait = false) -> bool {
        auto const raii = LogRAII_PS("readDataName "+dataName);
        return this->codices.contains(dataName) ? codices.at(dataName).read(info, data, isTriviallyCopyable) : false;
    }

    virtual auto readSpaceName(std::string const &spaceName, Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable, bool const shouldWait = false) -> bool {
        auto const raii = LogRAII_PS("readSpaceName "+spaceName);
        return this->codices.contains(spaceName) ?
            codices[spaceName].template visitFirst<PathSpaceTE>([&range, info, data, isTriviallyCopyable](auto &space){return space.read(range.next(), info, data, isTriviallyCopyable);}) :
            false;
    }

    virtual auto insertDataName(Path const &range, std::string const &dataName, Data const &data, Path const &coroResultPath) -> bool {
        auto const raii = LogRAII_PS("insertDataName "+dataName);
        assert(this->root!=nullptr);
        UnlockedToExclusiveLock upgraded(this->mutex);
        this->codices[dataName].insert(range.original(), coroResultPath, data, *this->root); // ToDo: What about recursive coroutines???
        return true;
    }

    virtual auto insertSpaceName(Path const &range, std::string const &spaceName, Data const &data, Path const &coroResultPath) -> bool {
        auto const raii = LogRAII_PS("insertSpaceName "+spaceName);
        UnlockedToExclusiveLock upgraded(this->mutex);
        if(!codices.contains(spaceName)) {
            codices[spaceName].insertSpace(PathSpaceTE(PathSpace{spaceName, this->root}));
            this->condition.notify_all();
        }
        return codices[spaceName].template visitFirst<PathSpaceTE>([&range, &data, this](auto &space){
            if(space.insert(range.next(), data)) {
                this->condition.notify_all();
                return true;
            }
            return false;
        });
    }

    private:
        std::unordered_map<std::string, Codex> codices;
        mutable std::condition_variable_any condition;
        mutable int listeners = 0;
        mutable UpgradableMutex mutex;
        PathSpaceTE *root=nullptr;
};
}