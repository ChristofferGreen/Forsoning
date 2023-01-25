#pragma once
#include <filesystem>
#include <map>
#include <condition_variable>
#include <shared_mutex>

#include "FSNG/Coroutine.hpp"
#include "FSNG/Path.hpp"
#include "FSNG/Forge/Ticket.hpp"
#include "FSNG/utils.hpp"

#include "nlohmann/json.hpp"

namespace FSNG {
class PathSpaceTE {
	struct concept_t {
		virtual      ~concept_t   ()                                                         = default;
		virtual auto operator==   (const concept_t &rhs) const -> bool                       = 0;
		virtual auto preDestruct_ ()                           -> void                       = 0;
		virtual auto copy_        ()                     const -> std::unique_ptr<concept_t> = 0;
		
		virtual auto toJSON_         ()                                                                             const  -> nlohmann::json             = 0;
		virtual auto insert_         (Path const &range, Data const &data, Path const &coroResultPath)                     -> bool                       = 0;
		virtual auto grab_           (Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                       = 0;
		virtual auto grabBlock_      (Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                       = 0;
		virtual auto read_           (Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                       = 0;
		virtual auto readBlock_      (Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                       = 0;
		virtual auto setRoot_        (PathSpaceTE *root)                                                                   -> void                       = 0;
		virtual auto removeCoroutine_(Path const &path, Ticket const &ticket)                                              -> bool                       = 0;
	};
public:
	template<typename T>
	static auto Create() -> PathSpaceTE {
		PathSpaceTE ps;
		ps.self = std::make_unique<model<T>>();
		ps.self->setRoot_(&ps);
		return ps;
	}
	PathSpaceTE() = default;
	template<typename T>
	PathSpaceTE(T x)                             : self(std::make_unique<model<T>>(std::move(x))) {this->self->setRoot_(this);}
	PathSpaceTE(PathSpaceTE const &rhs)          : self(rhs.self->copy_())                        {this->self->setRoot_(this);}
	PathSpaceTE(PathSpaceTE &&rhs)               : self(std::move(rhs.self))                      {this->self->setRoot_(this);}
	PathSpaceTE(std::unique_ptr<concept_t> self) : self(std::move(self))                          {this->self->setRoot_(this);}

	~PathSpaceTE() {
		if(this->self)
			this->self->preDestruct_();
	}

	auto operator= (PathSpaceTE const &rhs)       -> PathSpaceTE& {return *this = PathSpaceTE(rhs);}
	auto operator= (PathSpaceTE &&rhs) noexcept   -> PathSpaceTE& {
		this->self = std::move(rhs.self);
		return *this;
	};
	auto operator==(const PathSpaceTE &rhs) const -> bool         {
		if(!this->self && !rhs.self)
			return true;
		if(!this->self || !rhs.self)
			return false;
		return *this->self == *rhs.self;
	};

	auto toJSON()                                                                            const -> nlohmann::json      { return this->self->toJSON_(); }
	auto insert(Path const &range, Data const &data, Path const &coroResultPath="")                -> bool                { 
		if(auto ret = this->self->insert_(range, data, coroResultPath)) {
			this->grabWaitersMutex.lock_shared();
			if(this->grabWaiters.count(range))
				this->grabWaiters.at(range).second.notify_all();
			this->grabWaitersMutex.unlock_shared();
			return true;
		}
		return false;
	}
    auto grabBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                {
		return this->self->grabBlock_(range, info, data, isTriviallyCopyable);
	}
    auto grab(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                {
		return this->self->grab_(range, info, data, isTriviallyCopyable);
	}
	template<typename T>
	auto grab(Path const &path, Ticket const &ticket)                   -> bool {
		auto ret = this->self->removeCoroutine_(path, ticket);
		this->grabWaitersMutex.lock_shared();
		if(this->grabWaiters.count(path))
			this->grabWaiters.at(path).second.notify_all();
		this->grabWaitersMutex.unlock_shared();
		return ret;
	}
	template<typename T>
	auto grab(Path const &range)                                         -> std::optional<T> {
		T data;
		if(this->grab(range, &typeid(T), reinterpret_cast<void*>(&data), std::is_trivially_copyable<T>()))
			return data;
		return std::nullopt;
	}
	template<typename T>
    auto grabBlock(Path const &range) -> T {
		std::optional<T> val;
		while(!val) {
			this->grabWaitersMutex.lock();
			if(val = this->grab<T>(range); !val) {
				this->grabWaiters[range].first++;
				this->grabWaiters[range].second.wait(this->grabWaitersMutex);
				this->grabWaiters[range].first--;
				if(this->grabWaiters[range].first==0)
					this->grabWaiters.erase(range);
			}
			this->grabWaitersMutex.unlock();
		}
		return *val;
	}
    auto read(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
		return this->self->read_(range, info, data, isTriviallyCopyable);
	}
	template<typename T>
	auto read(Path const &range)                                         -> std::optional<T> {
		T data;
		if(this->read(range, &typeid(T), reinterpret_cast<void*>(&data), std::is_trivially_copyable<T>()))
			return data;
		return std::nullopt;
	}
    auto readBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
		return this->self->readBlock_(range, info, data, isTriviallyCopyable);
	}
	template<typename T>
	auto readBlock(Path const &range)                                         -> T {
		T data;
		this->readBlock(range, &typeid(T), reinterpret_cast<void*>(&data), std::is_trivially_copyable<T>());
		return data;
	}
	auto setRoot(PathSpaceTE *root) -> void {
		this->self->setRoot_(root);
	}
private:
	template<typename T> 
	struct model final : concept_t {
		model() = default;
		model(T x) : data(std::move(x)) {}
		auto operator==(const concept_t &rhs) const -> bool override { return this->data==reinterpret_cast<model<T> const&>(rhs).data; }
		auto copy_()                                                                                 const   -> std::unique_ptr<concept_t> override {return std::make_unique<model>(*this);}
		auto preDestruct_()                                                                                  -> void                       override {this->data.preDestruct();}

		auto toJSON_()                                                                               const   -> nlohmann::json             override {return this->data.toJSON();}
		auto insert_(Path const &range, Data const &d, Path const &coroResultPath)                           -> bool                       override {return this->data.insert(range, d, coroResultPath);}
		auto grab_(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable)      -> bool                       override {return this->data.grab(range, info, data, isTriviallyCopyable);}
		auto grabBlock_(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                       override {return this->data.grabBlock(range, info, data, isTriviallyCopyable);}
		auto read_(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable)      -> bool                       override {return this->data.read(range, info, data, isTriviallyCopyable);}
		auto readBlock_(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                       override {return this->data.readBlock(range, info, data, isTriviallyCopyable);}
		auto setRoot_(PathSpaceTE *root)                                                                     -> void                       override {this->data.setRoot(root);}
		auto removeCoroutine_(Path const &path, Ticket const &ticket)                                        -> bool                       override {return this->data.removeCoroutine(path, ticket);}

		T data;
	};
	std::unique_ptr<concept_t> self;
	std::map<Path, std::pair<int, std::condition_variable_any>> grabWaiters;
	std::shared_mutex grabWaitersMutex;
};
}