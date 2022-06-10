#pragma once
#include <filesystem>

#include "FSNG/Coroutine.hpp"
#include "FSNG/Path.hpp"
#include "FSNG/Forge/TaskProcessor.hpp"
#include "FSNG/utils.hpp"

#include "nlohmann/json.hpp"

namespace FSNG {
class PathSpaceTE {
	struct concept_t {
		virtual ~concept_t() = default;
		virtual auto operator==(const concept_t &rhs) const -> bool = 0;
		
		virtual auto copy_()                                                                                  const  -> std::unique_ptr<concept_t> = 0;
		virtual auto toJSON_()                                                                                const  -> nlohmann::json             = 0;
		virtual auto setProcessor_(std::shared_ptr<TaskProcessor> const &processor)                                  -> void                       = 0;
		virtual auto insert_(Path const &range, Data const &data)                                                    -> bool                       = 0;
		virtual auto grab_(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable)      -> bool                       = 0;
		virtual auto grabBlock_(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                       = 0;
        /*virtual auto popFrontData_()                                                                                        -> std::optional<DataType>    = 0;
		virtual auto grab_(std::filesystem::path const &path, PathIterConstPair const &iters)                               -> std::optional<PathSpaceTE> = 0;
		virtual auto grabBlock_(std::filesystem::path const &path)                                                          -> std::optional<PathSpaceTE> = 0;
		virtual auto grabBlock_(std::filesystem::path const &path, PathIterConstPair const &iters)                          -> std::optional<PathSpaceTE> = 0;*/
	};
public:
	PathSpaceTE() = default;
	template<typename T>
	PathSpaceTE(T x)                             : self(std::make_unique<model<T>>(std::move(x))) {}
	PathSpaceTE(PathSpaceTE const &rhs)          : self(rhs.self->copy_())                        {}
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
	auto setProcessor(std::shared_ptr<TaskProcessor> const &processor)                             -> void                { return this->self->setProcessor_(processor); }
	auto insert(Path const &range, Data const &data)                                               -> bool                { return this->self->insert_(range, data); }
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
    auto grabBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                {
		return this->self->grabBlock_(range, info, data, isTriviallyCopyable);
	}
	template<typename T>
	auto grabBlock(Path const &range)                                         -> std::optional<T> {
		T data;
		if(this->grabBlock(range, &typeid(T), reinterpret_cast<void*>(&data), std::is_trivially_copyable<T>()))
			return data;
		return std::nullopt;
	}
	/*auto insert(std::filesystem::path const &path, PathIterConstPair const &iters, DataType const &data)       -> bool { return this->self->insert_(path, iters, data); }

    template<typename T>
    auto grab(std::filesystem::path const &path) -> std::optional<T> {
        if(auto val = this->self->grab_(path)) {
            if constexpr(std::is_same<T, PathSpaceTE>::value)
                return val.value();
            else if(auto data = val.value().popFrontData())
                return std::get<T>(data.value());
        }
        return {};
    }
    
    auto grab(std::filesystem::path const &path, PathIterConstPair const &iters) -> std::optional<PathSpaceTE> {
        return this->self->grab_(path, iters);
    }

    template<typename T>
	auto grabBlock(std::filesystem::path const &path) -> std::optional<T> { 
        if(auto val = this->self->grabBlock_(path)) {
            if constexpr(std::is_same<T, PathSpaceTE>::value)
                return val.value();
            else if(auto data = val.value().popFrontData())
                return std::get<T>(data.value());
        }
        return {};
    }

    auto grabBlock(std::filesystem::path const &path, PathIterConstPair const &iters) -> std::optional<PathSpaceTE> {
        return this->self->grabBlock_(path, iters);
    }

	auto popFrontData() -> std::optional<DataType> { 
        return this->self->popFrontData_();
    }*/
private:
	template<typename T> 
	struct model final : concept_t {
		model(T x) : data(std::move(x)) {}
		auto operator==(const concept_t &rhs) const -> bool override { return this->data==reinterpret_cast<model<T> const&>(rhs).data; }

		auto copy_()                                                                                 const   -> std::unique_ptr<concept_t> override {return std::make_unique<model>(*this);}
		auto toJSON_()                                                                               const   -> nlohmann::json             override {return this->data.toJSON();}
		auto setProcessor_(std::shared_ptr<TaskProcessor> const &processor)                                  -> void                       override {return this->data.setProcessor(processor);}
		auto insert_(Path const &range, Data const &d)                                                       -> bool                       override {return this->data.insert(range, d);}
		auto grab_(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable)      -> bool                       override {return this->data.grab(range, info, data, isTriviallyCopyable);}
		auto grabBlock_(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool                       override {return this->data.grabBlock(range, info, data, isTriviallyCopyable);}
		/*auto insert_(std::filesystem::path const &path, PathIterConstPair const &iters, DataType const &d)      -> bool                           override {return this->data.insert(path, iters, d);}
        auto popFrontData_()                                                                                    -> std::optional<DataType>        override {return this->data.popFrontData();}
		auto grab_(std::filesystem::path const &path)                                                           -> std::optional<PathSpaceTE>     override {return this->data.grab(path);}
		auto grab_(std::filesystem::path const &path, PathIterConstPair const &iters)                           -> std::optional<PathSpaceTE>     override {return this->data.grab(path, iters);}
		auto grabBlock_(std::filesystem::path const &path)                                                      -> std::optional<PathSpaceTE>     override {return this->data.grabBlock(path);}
		auto grabBlock_(std::filesystem::path const &path, PathIterConstPair const &iters)                      -> std::optional<PathSpaceTE>     override {return this->data.grabBlock(path, iters);}*/
		
		T data;
	};
	std::unique_ptr<concept_t> self;
};
}