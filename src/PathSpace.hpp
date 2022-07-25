#pragma once
#include "FSNG/Coroutine.hpp"
#include "FSNG/Codex.hpp"
#include "FSNG/Data.hpp"
#include "FSNG/Path.hpp"
#include "FSNG/PathSpaceTE.hpp"
#include "FSNG/Security.hpp"
#include "FSNG/Forge/Eschelon.hpp"
#include "FSNG/Forge/Hearth.hpp"
#include "FSNG/Forge/UpgradableMutex.hpp"
#include "FSNG/utils.hpp"

#include "nlohmann/json.hpp"

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

#ifdef LOG_PATH_SPACE
#define LOG_PS LOG
#else
#define LOG_PS(...)
#define LogRAII(...) 0
#endif

namespace FSNG {
struct PathSpace {
    PathSpace() = default;
    PathSpace(const PathSpace &rhs) {
        UnlockedToExclusiveLock lock(this->mutex);
        UnlockedToSharedLock lockRHS(rhs.mutex);
        this->codices = rhs.codices;
    }

    auto operator==(PathSpace const &rhs) const -> bool { return this->codices==rhs.codices; }

    auto grab(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        auto const raii = LogRAII("PathSpace::grab "+range.dataName());
        UnlockedToUpgradedLock lock(this->mutex);
        if(range.isAtData())
            return this->grabDataName(range.dataName(), info, data, isTriviallyCopyable);
        return this->grabSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable);
    }

    auto grabBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        auto const raii = LogRAII("PathSpace::grabBlock "+range.dataName());
        bool found = false;
        if(range.isAtRoot()) {
            while(!found) {
                UnlockedToUpgradedLock lock(this->mutex);
                bool const shouldWait = true;
                found = range.isAtData() ? this->grabDataName(range.dataName(), info, data, isTriviallyCopyable, shouldWait) :
                                           this->grabSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable, shouldWait);
            }
        } else {
            UnlockedToUpgradedLock lock(this->mutex);
            if(range.isAtData())
                return this->grabDataName(range.dataName(), info, data, isTriviallyCopyable);
            return this->grabSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable);
        }
        return found;
    }

    auto read(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        auto const raii = LogRAII("PathSpace::read "+range.dataName());
        UnlockedToSharedLock lock(this->mutex);
        if(range.isAtData())
            return this->readDataName(range.dataName(), info, data, isTriviallyCopyable);
        return this->readSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable);
    }

    auto readBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        auto const raii = LogRAII("PathSpace::readBlock "+range.dataName());
        bool found = false;
        if(range.isAtRoot()) {
            while(!found) {
                UnlockedToSharedLock lock(this->mutex);
                bool const shouldWait = true;
                found = range.isAtData() ? this->readDataName(range.dataName(), info, data, isTriviallyCopyable, shouldWait) :
                                           this->readSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable, shouldWait);
            }
        } else {
            UnlockedToSharedLock lock(this->mutex);
            if(range.isAtData())
                return this->readDataName(range.dataName(), info, data, isTriviallyCopyable);
            return this->readSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable);
        }
        return found;
    }

    virtual auto insert(Path const &range, Data const &data) -> bool {
        auto const raii = LogRAII("PathSpace::insert "+range.dataName());
        if(range.isAtData())
            return this->insertDataName(range.dataName(), data);
        return this->insertSpaceName(range, range.spaceName().value(), data);
    }

    virtual auto toJSON() const -> nlohmann::json {
        auto const raii = LogRAII("PathSpace::toJSON");
        nlohmann::json json;
        UnlockedToSharedLock lock(this->mutex);
        for(auto const &p : codices)
            json[p.first] = p.second.toJSON();
        return json;
    }

private:
    virtual auto grabDataName(std::string const &dataName, std::type_info const *info, void *data, bool isTriviallyCopyable, bool const shouldWait = false) -> bool {
        auto const raii = LogRAII("PathSpace::grabDataName "+dataName);
        if(this->codices.contains(dataName)) {
            UpgradedToExclusiveLock upgraded(this->mutex);
            bool const found = codices.at(dataName).grab(info, data, isTriviallyCopyable);
            if(!found && shouldWait) {
                auto u = UpgradableMutexWaitableWrapper(this->mutex);
                auto const raii = LogRAII("PathSpace::grabDataName condition wait (UpgradedToExclusiveLock)");
                this->condition.wait(u);
            }
            return found;
        }
        return false;
    }

    virtual auto grabSpaceName(std::string const &spaceName, Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable, bool const shouldWait = false) -> bool {
        auto const raii = LogRAII("PathSpace::grabSpaceName "+dataName);
        if(this->codices.contains(spaceName)) {
            UpgradedToExclusiveLock upgraded(this->mutex);
            bool const found = codices[spaceName].template visitFirst<PathSpaceTE>([&range, info, data, isTriviallyCopyable](auto &space){return space.grab(range.next(), info, data, isTriviallyCopyable);});
            if(!found && shouldWait) {
                auto u = UpgradableMutexWaitableWrapper(this->mutex);
                this->condition.wait(u);
            }
            return found;
        }
        return false;
    }

    virtual auto readDataName(std::string const &dataName, std::type_info const *info, void *data, bool isTriviallyCopyable, bool const shouldWait = false) -> bool {
        auto const raii = LogRAII("PathSpace::readDataName "+dataName);
        if(this->codices.contains(dataName))
            return codices.at(dataName).read(info, data, isTriviallyCopyable);
        return false;
    }

    virtual auto readSpaceName(std::string const &spaceName, Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable, bool const shouldWait = false) -> bool {
        auto const raii = LogRAII("PathSpace::readSpaceName "+spaceName);
        if(this->codices.contains(spaceName))
            return codices[spaceName].template visitFirst<PathSpaceTE>([&range, info, data, isTriviallyCopyable](auto &space){return space.read(range.next(), info, data, isTriviallyCopyable);});
        return false;
    }

    virtual auto insertDataName(std::string const &dataName, Data const &data) -> bool {
        auto const raii = LogRAII("PathSpace::insertDataName "+dataName);
        UnlockedToExclusiveLock upgraded(this->mutex);
        this->codices[dataName].insert(data, [this, dataName](Data const &coroData){
            auto const raii = LogRAII("PathSpace::insertDataName codex insert");
            UnlockedToExclusiveLock upgraded(this->mutex);
            codices[dataName].insert(coroData, [](Data const &data){});
            this->condition.notify_all();
        }); // ToDo:: What about recursive coroutines???
        return true;
    }

    virtual auto insertSpaceName(Path const &range, std::string const &spaceName, Data const &data) -> bool {
        auto const raii = LogRAII("PathSpace::insertSpaceName "+spaceName);
        UnlockedToExclusiveLock upgraded(this->mutex);
        if(!codices.contains(spaceName)) {
            codices[spaceName].insert(PathSpaceTE(PathSpace{}), [](Data const &data){});
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
        mutable UpgradableMutex mutex;
};
}