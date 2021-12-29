#pragma once

#include <functional>
#include <filesystem>
#include <unordered_map>
#include <optional>
#include <variant>

namespace Forsoning {

struct Security {
    bool insert = false;
    bool grab   = false;
};
using SecFunT = std::function<Security(std::filesystem::path const &path)>;

struct Data {
    Data() = default;
    Data(int d) : var(d) {}

    template<typename T>
    auto to() const -> std::optional<T> { 
        if(this->var.index()!=std::variant_npos)
            return std::get<T>(this->var);
        return {};
    }

    std::variant<int> var;
};

struct PathSpaceTE {
	PathSpaceTE() = default;
	template<typename T>
	PathSpaceTE(T x)                    : self(std::make_unique<model<T>>(std::move(x))) {}
	PathSpaceTE(PathSpaceTE const &rhs) : self(rhs.self->copy_())                        {}

	PathSpaceTE& operator=(PathSpaceTE const &rhs) {return *this = PathSpaceTE(rhs);}
	PathSpaceTE& operator=(PathSpaceTE&&) noexcept = default;

	auto insert(std::filesystem::path const &path, Data const &data) -> void                { this->self->insert_(path, data); }
	auto grab(std::filesystem::path const &path)                     -> std::optional<Data> { return this->self->grab_(path); }
private:
	struct concept_t {
		virtual ~concept_t() = default;
		
		virtual auto copy_()                                                      const -> std::unique_ptr<concept_t> = 0;
		virtual auto insert_(std::filesystem::path const &path, Data const &data)       -> void                       = 0;
		virtual auto grab_(std::filesystem::path const &path)                           -> std::optional<Data>        = 0;
	};
	template<typename T> 
	struct model final : concept_t {
		model(T x) : data(std::move(x)) {}

		auto copy_() const -> std::unique_ptr<concept_t> override {return std::make_unique<model>(*this);}
		
		auto insert_(std::filesystem::path const &path, Data const &d)       -> void                override {return this->data.insert(path, d);}
		auto grab_(std::filesystem::path const &path)                        -> std::optional<Data> override {return this->data.grab(path);}
		
		T data;
	};
	std::unique_ptr<concept_t> self;
};

struct PathSpace {
    protected:
    friend PathSpaceTE;
    auto insert(std::filesystem::path const &path, Data const &data) -> void {
        this->data.insert(std::make_pair(path.filename().string(),  data));
    };
    auto grab(std::filesystem::path const &path) -> std::optional<Data> {
        std::string const name = path.filename().string();
        if(this->data.count(name))
            return this->data.extract(name).mapped();
        return {};
    };
    std::unordered_multimap<std::string, Data> data;
};

struct View {
    View(PathSpaceTE space, SecFunT const &security) : space(space), security(security) {};
    
    auto insert(std::filesystem::path const &path, Data const &data) -> void {
        if(this->security(path).insert)
            this->space.insert(path, data);
    };
    template<typename T>
    auto grab(std::filesystem::path const &path) -> std::optional<T> { 
        if(this->security(path).grab) {
            if(auto const val = this->space.grab(path))
                return val.value().to<T>();
        }
        return {};
    };

    private:
        PathSpaceTE space;
        SecFunT security;
};

}