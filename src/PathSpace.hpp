#pragma once

#include <condition_variable>
#include <functional>
#include <filesystem>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <optional>
#include <variant>

namespace Forsoning {

namespace Security {
    enum struct Type {
        Grab = 0,
        Insert
    };
    using FunT = std::function<bool(std::filesystem::path const &path, Type const &sec)>;
    namespace Policy {
        inline const FunT AlwaysAllow = [](std::filesystem::path const &path, Type const &sec){ return true; };
    }
}

class PathSpaceTE;
struct Data {
    Data() = default;
    Data(int d) : var(d) {}
    Data(std::unique_ptr<PathSpaceTE> ptr) : var(std::move(ptr)) {}
    Data(Data const &data) {
        if(std::holds_alternative<int>(data.var)) {
            this->var = std::get<int>(data.var);
        } else if(std::holds_alternative<std::unique_ptr<PathSpaceTE>>(data.var)) {
            this->var = std::make_unique<PathSpaceTE>(*std::get<std::unique_ptr<PathSpaceTE>>(data.var));
        }
    }

    template<typename T>
    auto to() const -> std::optional<T> { 
        if(this->var.index()!=std::variant_npos)
            return std::get<T>(this->var);
        return {};
    }

    template<typename T>
    auto toMove() -> std::optional<T> { 
        if(this->var.index()!=std::variant_npos)
            return std::move(std::get<T>(this->var));
        return {};
    }

    std::variant<int, std::unique_ptr<PathSpaceTE>> var;
};

class PathSpaceTE {
	struct concept_t {
		virtual ~concept_t() = default;
		
		virtual auto copy_()                                                      const -> std::shared_ptr<concept_t> = 0;
		virtual auto size_()                                                      const -> int                        = 0;
		virtual auto insert_(std::filesystem::path const &path, Data const &data)       -> void                       = 0;
		virtual auto grab_(std::filesystem::path const &path)                           -> std::optional<Data>        = 0;
		virtual auto grabBlock_(std::filesystem::path const &path)                      -> std::optional<Data>        = 0;
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
    template<typename T>
	auto grab(std::filesystem::path const &path)                           -> std::optional<T> {
        if(auto val = this->self->grab_(path))
            if(auto conv = val.value().toMove<T>())
                return std::move(conv.value());
        return {};
    }
    template<typename T>
	auto grabBlock(std::filesystem::path const &path)                      -> std::optional<T> { 
        if(auto val = this->self->grabBlock_(path))
            if(auto conv = val.value().toMove<T>())
                return conv.value();
        return {};
    }
private:
	template<typename T> 
	struct model final : concept_t {
		model(T x) : data(std::move(x)) {}

		auto copy_()                                                   const -> std::shared_ptr<concept_t> override {return std::make_shared<model>(*this);}
		auto size_()                                                   const -> int                        override {return this->data.size();}
		auto insert_(std::filesystem::path const &path, Data const &d)       -> void                       override {return this->data.insert(path, d);}
		auto grab_(std::filesystem::path const &path)                        -> std::optional<Data>        override {return this->data.grab(path);}
		auto grabBlock_(std::filesystem::path const &path)                   -> std::optional<Data>        override {return this->data.grabBlock(path);}
		
		T data;
	};
	std::shared_ptr<concept_t> self;
};

auto nbr_elements(std::filesystem::path const &path) -> int {
    int elements = 0;
    for(auto const &element : path)
        elements++;
    return elements;
}

template<typename T=std::unordered_multimap<std::string, Data>>
struct PathSpace {
    PathSpace() = default;
    PathSpace(PathSpace const &ps) : data(ps.data) {}

    virtual auto insert(std::filesystem::path const &path, Data const &data) -> void {
        this->createPathTo(path);
        this->access(path, [](PathSpaceTE &space){space.});
        this->insert(++path.begin(), path.end(), data);
    };

    virtual auto grab(std::filesystem::path const &path) -> std::optional<Data> {
        return this->grab(++path.begin(), path.end());
    };

    virtual auto grabBlock(std::filesystem::path const &path) -> std::optional<Data> {
        return this->grabBlock(++path.begin(), path.end());
    };

    virtual auto size() const -> int {
        std::shared_lock<std::shared_mutex> lock(this->mut); // read
        return this->data.size();
    }

private:
    virtual auto insert(std::filesystem::path::const_iterator const &iter, std::filesystem::path::const_iterator const &end, Data const &data) -> void {

        
        std::lock_guard<std::shared_mutex> lock(this->mut); // write
        auto iterNext = iter++;
        iterNext++;
        if(iterNext==end) {
            this->data.insert(std::make_pair(*iter, data));
            this->cv.notify_all();
        } else {
            if(this->data.count(*iter)>0) {

            } else {
                auto node = PathSpace{};
                node.insert(iterNext, end, data);
                this->data.insert(std::make_pair(*iter, std::make_unique<PathSpaceTE>(PathSpaceTE{std::move(node)})));
                this->cv.notify_all();
            }
        }
    };
    
    virtual auto grab(std::filesystem::path::const_iterator const &iter, std::filesystem::path::const_iterator const &end) -> std::optional<Data> {
        std::string const name = *iter;
        std::lock_guard<std::shared_mutex> lock(this->mut); // write
        if(this->data.count(name))
            return this->data.extract(name).mapped();
        return {};
    };

    virtual auto grabBlock(std::filesystem::path::const_iterator const &iter, std::filesystem::path::const_iterator const &end) -> std::optional<Data> {
        std::string const name = *iter;
        std::unique_lock<std::shared_mutex> lock(this->mut); // write
        while(!this->data.count(name))
            this->cv.wait(lock);
        return this->data.extract(name).mapped();
    };

    mutable std::shared_mutex mut;
    mutable std::condition_variable_any cv;
    T data;
};

template<typename T>
auto to(std::optional<Data> const &data) -> std::optional<T> {
    if(data)
        if(auto const val = data.value().to<T>())
            return val;
    return {};
}

template<typename T>
struct View {
    View(T const &space, Security::FunT const &security) : space(space.link()), security(security) {};
    
    auto insert(std::filesystem::path const &path, Data const &data) -> void {
        if(this->security(path, Security::Type::Insert))
            this->space.insert(path, data);
    };

    template <typename U>
    auto grab(std::filesystem::path const &path) -> std::optional<U> { 
        if(this->security(path, Security::Type::Grab)) {
            if(auto const val = this->space.template grab<U>(path))
                return val.value();
        }
        return {};
    };

    template <typename U>
    auto grabBlock(std::filesystem::path const &path) -> std::optional<U> {
        if(this->security(path, Security::Type::Grab)) {
            if(auto const val = this->space.template grabBlock<U>(path))
                return val.value();
        }
        return {};
    };

    auto size() const -> int {
        return this->space.size();
    }

private:
    T space;
    Security::FunT security;
};

}