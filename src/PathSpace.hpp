#pragma once

#include <functional>
#include <filesystem>
#include <unordered_map>
#include <optional>
#include <variant>

namespace Forsoning {

enum struct SecurityType {
    Grab = 0,
    Insert
};
using SecFunT = std::function<bool(std::filesystem::path const &path, SecurityType const &sec)>;
inline const SecFunT SecurityAlwaysAllow = [](std::filesystem::path const &path, SecurityType const &sec){ return true; };

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

class PathSpaceTE {
	struct concept_t {
		virtual ~concept_t() = default;
		
		virtual auto copy_()                                                      const -> std::shared_ptr<concept_t> = 0;
		virtual auto size_()                                                      const -> int                       = 0;
		virtual auto insert_(std::filesystem::path const &path, Data const &data)       -> void                       = 0;
		virtual auto grab_(std::filesystem::path const &path)                           -> std::optional<Data>        = 0;
	};
public:
	PathSpaceTE() = default;
	template<typename T>
	PathSpaceTE(T x)                             : self(std::make_shared<model<T>>(std::move(x))) {}
	PathSpaceTE(PathSpaceTE const &rhs)          : self(rhs.self->copy_())                        {}
	PathSpaceTE(std::shared_ptr<concept_t> self) : self(self)                                     {}

	auto operator=(PathSpaceTE const &rhs) -> PathSpaceTE& {return *this = PathSpaceTE(rhs);}
	auto operator=(PathSpaceTE&&) noexcept -> PathSpaceTE& = default;

	auto link()                                                      const -> PathSpaceTE         { return this->self; }
	auto size()                                                      const -> int                 { return this->self->size_(); }
	auto insert(std::filesystem::path const &path, Data const &data)       -> void                { this->self->insert_(path, data); }
	auto grab(std::filesystem::path const &path)                           -> std::optional<Data> { return this->self->grab_(path); }
private:
	template<typename T> 
	struct model final : concept_t {
		model(T x) : data(std::move(x)) {}

		auto copy_()                                                   const -> std::shared_ptr<concept_t> override {return std::make_shared<model>(*this);}
		auto size_()                                                   const -> int                        override {return this->data.size();}
		auto insert_(std::filesystem::path const &path, Data const &d)       -> void                       override {return this->data.insert(path, d);}
		auto grab_(std::filesystem::path const &path)                        -> std::optional<Data>        override {return this->data.grab(path);}
		
		T data;
	};
	std::shared_ptr<concept_t> self;
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

    auto size() const -> int {
        return this->data.size();
    }

    std::unordered_multimap<std::string, Data> data;
};

struct View {
    View(PathSpaceTE const &space, SecFunT const &security) : space(space.link()), security(security) {};
    
    auto insert(std::filesystem::path const &path, Data const &data) -> void {
        if(this->security(path, SecurityType::Insert))
            this->space.insert(path, data);
    };

    template<typename T>
    auto grab(std::filesystem::path const &path) -> std::optional<T> { 
        if(this->security(path, SecurityType::Grab)) {
            if(auto const val = this->space.grab(path))
                return val.value().to<T>();
        }
        return {};
    };

    auto size() const -> int {
        return this->space.size();
    }

    private:
        PathSpaceTE space;
        SecFunT security;
};

}