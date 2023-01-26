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
    ~PathSpace() {
        Forge::instance()->clearBlock(*this->root);
    }

    auto operator==(PathSpace const &rhs) const -> bool { return this->codices==rhs.codices; }

    auto removeCoroutine(Path const &range, Ticket const &ticket) -> bool {
        bool ret = false;
        if(range.isAtData()) {
            UnlockedToExclusiveLock upgraded(this->mutex);
            if(this->codices.contains(range.dataName())) {
                this->codices[range.dataName()].removeCoroutine(ticket);
                if(this->codices[range.dataName()].empty())
                    this->codices.erase(range.dataName());
                ret = true;
            }
        } else {
            UnlockedToUpgradedLock lock(this->mutex);
            auto const spaceName = range.spaceName().value();
            if(this->codices.contains(spaceName)) {
                UpgradedToExclusiveLock upgraded(this->mutex);
                ret = codices[spaceName].template visitFirst<PathSpaceTE>([&range, ticket](auto &space){return space.template grab<Coroutine>(range.next(), ticket);});
            }
        }
		this->grabWaitersMutex.lock_shared();
		if(this->grabWaiters.count(range))
			this->grabWaiters.at(range).second.notify_all();
		this->grabWaitersMutex.unlock_shared();
		return ret;
    }

    auto grabBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
		bool ret = false;
		while(!ret) {
			this->grabWaitersMutex.lock();
			if(ret = this->grab(range, info, data, isTriviallyCopyable); !ret) {
				this->grabWaiters[range].first++;
				this->grabWaiters[range].second.wait(this->grabWaitersMutex);
				this->grabWaiters[range].first--;
				if(this->grabWaiters[range].first==0)
					this->grabWaiters.erase(range);
			}
			this->grabWaitersMutex.unlock();
		}
		return ret;
    }

    auto grab(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        auto const raii = LogRAII_PS("grab "+range.dataName());
        return range.isAtData() ? this->grabDataName(range.dataName(), info, data, isTriviallyCopyable) :
                                  this->grabSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable);
    }

    auto read(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        auto const raii = LogRAII_PS("read "+range.dataName());
        return range.isAtData() ?  this->readDataName(range.dataName(), info, data, isTriviallyCopyable) :
                                   this->readSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable);
    }

    auto readBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        auto const raii = LogRAII_PS("readBlock "+range.dataName());
        if(range.isAtRoot()) {
            bool found = false;
            while(!found) {
                bool const shouldWait = true;
                found = range.isAtData() ? this->readDataName(range.dataName(), info, data, isTriviallyCopyable, shouldWait) :
                                           this->readSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable, shouldWait);
            }
            return found;
        } else {
            return range.isAtData() ? this->readDataName(range.dataName(), info, data, isTriviallyCopyable) :
                                      this->readSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable);
        }
    }

    virtual auto insert(Path const &range, Data const &data, Path const &coroResultPath="") -> bool {
        auto const raii = LogRAII_PS("insert "+range.dataName());
        if(range==coroResultPath && range==Path("")) {
            this->root = data.as<PathSpaceTE*>();
            return false;
        }
        auto const ret = range.isAtData() ? this->insertDataName(range, range.dataName(), data, coroResultPath) :
                                            this->insertSpaceName(range, range.spaceName().value(), data, coroResultPath);
		if(ret) {
			this->grabWaitersMutex.lock_shared();
			if(this->grabWaiters.count(range))
				this->grabWaiters.at(range).second.notify_all();
			this->grabWaitersMutex.unlock_shared();
		}
		return ret;
    }

    virtual auto toJSON() const -> nlohmann::json {
        auto const raii = LogRAII_PS("toJSON");
        nlohmann::json json;
        UnlockedToSharedLock lock(this->mutex);
        for(auto const &p : codices)
            json[p.first] = p.second.toJSON();
        return json;
    }

private:
    virtual auto grabDataName(std::string const &dataName, std::type_info const *info, void *data, bool isTriviallyCopyable, bool const shouldWait = false) -> bool {
        auto const raii = LogRAII_PS("grabDataName "+dataName);
        UnlockedToUpgradedLock lock(this->mutex);
        bool found = false;
        if(this->codices.contains(dataName)) {
            UpgradedToExclusiveLock upgradedLock(this->mutex);
            found = codices.at(dataName).grab(info, data, isTriviallyCopyable);
            if(codices.at(dataName).empty())
                codices.erase(dataName);
        }
        return found;
    }

    virtual auto grabSpaceName(std::string const &spaceName, Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        auto const raii = LogRAII_PS("grabSpaceName "+spaceName);
        UnlockedToUpgradedLock lock(this->mutex);
        if(this->codices.contains(spaceName)) {
            bool const found = codices[spaceName].template visitFirst<PathSpaceTE>([&range, info, data, isTriviallyCopyable](auto &space){
                return space.grab(range.next(), info, data, isTriviallyCopyable);
            });
            return found;
        }
        return false;
    }

    virtual auto readDataName(std::string const &dataName, std::type_info const *info, void *data, bool isTriviallyCopyable, bool const shouldWait = false) -> bool {
        auto const raii = LogRAII_PS("readDataName "+dataName);
        UnlockedToSharedLock lock(this->mutex);
        return this->codices.contains(dataName) ? codices.at(dataName).read(info, data, isTriviallyCopyable) : false;
    }

    virtual auto readSpaceName(std::string const &spaceName, Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable, bool const shouldWait = false) -> bool {
        auto const raii = LogRAII_PS("readSpaceName "+spaceName);
        UnlockedToSharedLock lock(this->mutex);
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
        if(!codices.contains(spaceName))
            codices[spaceName].insertSpace(PathSpaceTE(PathSpace{spaceName, this->root}));
        return codices[spaceName].template visitFirst<PathSpaceTE>([&range, &data, this](auto &space){
            return space.insert(range.next(), data);
        });
    }

    private:
        std::unordered_map<std::string, Codex> codices;
        mutable UpgradableMutex mutex;
        PathSpaceTE *root=nullptr;
        std::map<Path, std::pair<int, std::condition_variable_any>> grabWaiters;
        std::shared_mutex grabWaitersMutex;
};
}