#pragma once
#include <filesystem>

#include "FSNG/Coroutine.hpp"
#include "FSNG/Path.hpp"
#include "FSNG/utils.hpp"

#include "nlohmann/json.hpp"

namespace FSNG {
class PathSpaceTE {
	struct concept_t {
		virtual ~concept_t() = default;
		virtual auto operator==(const concept_t &rhs) const -> bool = 0;
		
		virtual auto copy_     ()                                                                             const  -> std::unique_ptr<concept_t> = 0;
		virtual auto toJSON_   ()                                                                             const  -> nlohmann::json             = 0;
		virtual auto insert_   (Path const &range, Data const &data, Path const &coroResultPath)                     -> bool                       = 0;
		virtual auto grab_     (Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                       = 0;
		virtual auto grabBlock_(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                       = 0;
		virtual auto read_     (Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                       = 0;
		virtual auto readBlock_(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                       = 0;
		virtual auto setRoot_  (PathSpaceTE *root)                                                                   -> void                       = 0;
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
	PathSpaceTE(T x)                             : self(std::make_unique<model<T>>(std::move(x))) {}
	PathSpaceTE(PathSpaceTE const &rhs)          : self(rhs.self->copy_())                        {}
	PathSpaceTE(PathSpaceTE &&rhs)               : self(std::move(rhs.self))                      {}
	PathSpaceTE(std::unique_ptr<concept_t> self) : self(std::move(self))                          {}

	auto operator= (PathSpaceTE const &rhs)       -> PathSpaceTE& {return *this = PathSpaceTE(rhs);}
	auto operator= (PathSpaceTE&&) noexcept       -> PathSpaceTE& = default;
	auto operator==(const PathSpaceTE &rhs) const -> bool         {
		if(!this->self && !rhs.self)
			return true;
		if(!this->self || !rhs.self)
			return false;
		return *this->self == *rhs.self;
	};

	auto toJSON()                                                                            const -> nlohmann::json      { return this->self->toJSON_(); }
	auto insert(Path const &range, Data const &data, Path const &coroResultPath="")                -> bool                { return this->self->insert_(range, data, coroResultPath); }
    auto grab(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                {
		return this->self->grab_(range, info, data, isTriviallyCopyable);
	}
	template<typename T>
	auto grab(Path const &range)                                         -> std::optional<T> {
		T data;
		if(this->grab(range, &typeid(T), reinterpret_cast<void*>(&data), std::is_trivially_copyable<T>()))
			return data;
		return std::nullopt;
	}
    auto grabBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
		return this->self->grabBlock_(range, info, data, isTriviallyCopyable);
	}
	template<typename T>
	auto grabBlock(Path const &range)                                         -> T {
		T data;
		this->grabBlock(range, &typeid(T), reinterpret_cast<void*>(&data), std::is_trivially_copyable<T>());
		return data;
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
		auto toJSON_()                                                                               const   -> nlohmann::json             override {return this->data.toJSON();}
		auto insert_(Path const &range, Data const &d, Path const &coroResultPath)                           -> bool                       override {return this->data.insert(range, d, coroResultPath);}
		auto grab_(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable)      -> bool                       override {return this->data.grab(range, info, data, isTriviallyCopyable);}
		auto grabBlock_(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                       override {return this->data.grabBlock(range, info, data, isTriviallyCopyable);}
		auto read_(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable)      -> bool                       override {return this->data.read(range, info, data, isTriviallyCopyable);}
		auto readBlock_(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                       override {return this->data.readBlock(range, info, data, isTriviallyCopyable);}
		auto setRoot_(PathSpaceTE *root)                                                                     -> void                       override {this->data.setRoot(root);}

		T data;
	};
	std::unique_ptr<concept_t> self;
};
}