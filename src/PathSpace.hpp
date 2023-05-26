#pragma once
#include "FSNG/Coroutine.hpp"
#include "FSNG/Codex.hpp"
#include "FSNG/Data.hpp"
#include "FSNG/Path.hpp"
#include "FSNG/PathSpaceTE.hpp"
#include "FSNG/utils.hpp"

#include "nlohmann/json.hpp"

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

namespace FSNG {
struct PathSpace {
    PathSpace(std::string const &name="/") {};
    PathSpace(PathSpace const &rhs) {
        std::lock_guard<std::shared_mutex> lockGuard(this->mutex);
        std::shared_lock<std::shared_mutex> sharedLock(rhs.mutex);
        this->codices = rhs.codices;
    }
    PathSpace(PathSpace const &rhs, PathSpaceTE *root) : root(root) {
        std::lock_guard<std::shared_mutex> lockGuard(this->mutex);
        std::shared_lock<std::shared_mutex> sharedLock(rhs.mutex);
        this->codices = rhs.codices;
    }
    ~PathSpace() {
        Forge::instance()->clearBlock(*this->root);
    }

    auto operator==(PathSpace const &rhs) const -> bool { return this->codices==rhs.codices; }

    virtual auto grabBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
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

    virtual auto grab(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        return range.isAtData() ? this->grabDataName(range.dataName(), info, data, isTriviallyCopyable) :
                                  this->grabSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable);
    }

    virtual auto read(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        return range.isAtData() ?  this->readDataName(range.dataName(), info, data, isTriviallyCopyable) :
                                   this->readSpaceName(range.spaceName().value(), range, info, data, isTriviallyCopyable);
    }

    virtual auto readBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
		bool ret = false;
		while(!ret) {
			this->grabWaitersMutex.lock();
			if(ret = this->read(range, info, data, isTriviallyCopyable); !ret) {
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

    virtual auto insert(Path const &range, Data const &data, Path const &coroResultPath="") -> bool {
        if(range.empty()) {
            if(coroResultPath.empty()) {
                this->root = data.as<PathSpaceTE*>();
            } else {
                this->removeCoroutine(coroResultPath, data.as<Ticket>());
            }
            return true;
        }
        auto const ret = range.isAtData() ? this->insertDataName(range, range.dataName(), data, coroResultPath) :
                                            this->insertSpaceName(range, range.spaceName().value(), data, coroResultPath);
        if(!range.isAtRoot() && this->grabWaiters.count(range))
            assert(this->grabWaiters.at(range).first==0);
		if(ret && range.isAtRoot()) {
			this->grabWaitersMutex.lock_shared();
			if(this->grabWaiters.count(range))
				this->grabWaiters.at(range).second.notify_all();
			this->grabWaitersMutex.unlock_shared();
		}
		return ret;
    }

    virtual auto toJSON() const -> nlohmann::json {
        nlohmann::json json;
        std::shared_lock<std::shared_mutex> sharedLock(this->mutex);
        for(auto const &p : codices)
            json[p.first] = p.second.toJSON();
        return json;
    }

private:
    auto removeCoroutine(Path const &range, Ticket const &ticket) -> bool {
        bool ret = false;
        if(range.isAtData()) {
            std::lock_guard<std::shared_mutex> lockGuard(this->mutex);
            if(this->codices.contains(range.dataName())) {
                this->codices[range.dataName()].removeCoroutine(ticket);
                if(this->codices[range.dataName()].empty())
                    this->codices.erase(range.dataName());
                ret = true;
            }
        } else {
            std::lock_guard<std::shared_mutex> lockGuard(this->mutex);
            auto const spaceName = range.spaceName().value();
            if(this->codices.contains(spaceName)) {
                ret = codices[spaceName].template visitFirst<PathSpaceTE>([&range, ticket](auto &space){return space.insert("", ticket, range.next());});
            }
        }
        if(range.isAtRoot()) {
            this->grabWaitersMutex.lock_shared();
            if(this->grabWaiters.count(range))
                this->grabWaiters.at(range).second.notify_all();
            this->grabWaitersMutex.unlock_shared();
        }
		return ret;
    }
    
    virtual auto grabDataName(std::string const &dataName, std::type_info const *info, void *data, bool isTriviallyCopyable, bool const shouldWait = false) -> bool {
        std::lock_guard<std::shared_mutex> lockGuard(this->mutex);
        bool found = false;
        if(this->codices.contains(dataName)) {
            found = codices.at(dataName).grab(info, data, isTriviallyCopyable);
            if(codices.at(dataName).empty())
                codices.erase(dataName);
        }
        return found;
    }

    virtual auto grabSpaceName(std::string const &spaceName, Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        std::shared_lock<std::shared_mutex> sharedLock(this->mutex);
        if(this->codices.contains(spaceName)) {
            bool const found = codices[spaceName].template visitFirst<PathSpaceTE>([&range, info, data, isTriviallyCopyable](auto &space){
                return space.grab(range.next(), info, data, isTriviallyCopyable);
            });
            return found;
        }
        return false;
    }

    virtual auto readDataName(std::string const &dataName, std::type_info const *info, void *data, bool isTriviallyCopyable, bool const shouldWait = false) -> bool {
        std::shared_lock<std::shared_mutex> sharedLock(this->mutex);
        return this->codices.contains(dataName) ? codices.at(dataName).read(info, data, isTriviallyCopyable) : false;
    }

    virtual auto readSpaceName(std::string const &spaceName, Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable, bool const shouldWait = false) -> bool {
        std::shared_lock<std::shared_mutex> sharedLock(this->mutex);
        return this->codices.contains(spaceName) ?
            codices[spaceName].template visitFirst<PathSpaceTE>([&range, info, data, isTriviallyCopyable](auto &space){return space.read(range.next(), info, data, isTriviallyCopyable);}) :
            false;
    }

    virtual auto insertDataName(Path const &range, std::string const &dataName, Data const &data, Path const &coroResultPath) -> bool {
        assert(this->root!=nullptr);
        std::lock_guard<std::shared_mutex> lockGuard(this->mutex);
        this->codices[dataName].insert(range.original(), coroResultPath, data, *this->root); // ToDo: What about recursive coroutines???
        return true;
    }

    virtual auto insertSpaceName(Path const &range, std::string const &spaceName, Data const &data, Path const &coroResultPath) -> bool {
        std::lock_guard<std::shared_mutex> lockGuard(this->mutex);
        if(!codices.contains(spaceName))
            codices[spaceName].insertSpace(PathSpaceTE(PathSpace{spaceName, this->root}));
        return codices[spaceName].template visitFirst<PathSpaceTE>([&range, &data, this](auto &space){
            return space.insert(range.next(), data);
        });
    }

    private:
        std::unordered_map<std::string, Codex> codices;
        mutable std::shared_mutex mutex;
        PathSpaceTE *root=nullptr;
        std::map<Path, std::pair<int, std::condition_variable_any>> grabWaiters;
        std::shared_mutex grabWaitersMutex;
};

struct PathSpace2 {
    PathSpace2() = default;
    /*PathSpace2(const PathSpace2& other) : member(other.member ? std::make_unique<PathSpace2>(*other.member) : nullptr) {}
 
    PathSpace2& operator=(const PathSpace2& other) {
        this->member = other.member ? std::make_unique<PathSpace2>(*other.member) : nullptr;
        return *this;
    }*/

    virtual auto insert(Path const &range, InReference const &data) -> bool {
        return this->codices[range.dataName()].insert(data);
    }

    virtual auto toJSON() const -> nlohmann::json {
        nlohmann::json json;
        std::shared_lock<std::shared_mutex> sharedLock(this->mutex);
        for(auto const &p : this->codices)
            json[p.first] = p.second.toJSON();
        return json;
    }

protected:
    std::unordered_map<std::string, Codex2> codices;
    PathSpace2 *root = nullptr;
    mutable std::shared_mutex mutex;
};

}