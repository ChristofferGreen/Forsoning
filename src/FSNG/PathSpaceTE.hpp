#pragma once
#include <filesystem>

#include "Coro.hpp"
#include "Path.hpp"

#include "nlohmann/json.hpp"

namespace FSNG {
class PathSpaceTE {
	struct concept_t {
		virtual ~concept_t() = default;
		
		virtual auto copy_()                                                                                          const -> std::unique_ptr<concept_t> = 0;
		virtual auto toJSON_()                                      const -> nlohmann::json                        = 0;
		virtual auto insert_(Path const &path, Data     const &data)                                       -> bool                       = 0;
		/*virtual auto insert_(std::filesystem::path const &path, std::function<Coro<DataType>()> const &fun)                 -> bool                       = 0;
        virtual auto insert_(std::filesystem::path const &path, PathIterConstPair const &iters, DataType const &data)       -> bool                       = 0;
		virtual auto popFrontData_()                                                                                        -> std::optional<DataType>    = 0;
		virtual auto grab_(std::filesystem::path const &path)                                                               -> std::optional<PathSpaceTE> = 0;
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

	auto operator=(PathSpaceTE const &rhs) -> PathSpaceTE& {return *this = PathSpaceTE(rhs);}
	auto operator=(PathSpaceTE&&) noexcept -> PathSpaceTE& = default;

	auto toJSON()                                          const -> nlohmann::json  { return this->self->toJSON_(); }
	auto insert(Path const &path, Data const &data)                                           -> bool { return this->self->insert_(path, data); }
	/*auto insert(std::filesystem::path const &path, std::function<Coro<DataType>()> const &fun)                 -> bool { return this->self->insert_(path, fun); }
	auto insert(std::filesystem::path const &path, PathIterConstPair const &iters, DataType const &data)       -> bool { return this->self->insert_(path, iters, data); }

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

		auto copy_()                                                                                      const -> std::unique_ptr<concept_t>     override {return std::make_unique<model>(*this);}
		auto toJSON_()                                         const -> nlohmann::json                            override {return this->data.toJSON();}
		auto insert_(Path const &path, Data const &d)                                          -> bool                           override {return this->data.insert(path, d);}
		/*auto insert_(std::filesystem::path const &path, std::function<Coro<DataType>()> const &fun)             -> bool                           override {return this->data.insert(path, fun);}
		auto insert_(std::filesystem::path const &path, PathIterConstPair const &iters, DataType const &d)      -> bool                           override {return this->data.insert(path, iters, d);}
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