#pragma once

#include <condition_variable>
#include <functional>
#include <filesystem>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <variant>

namespace Forsoning {

using DataType             = std::variant<int, std::string>;
using PathIterConst        = std::filesystem::path::const_iterator;
using PathIterConstPair    = std::pair<PathIterConst, PathIterConst>;
using OptPathIterConstPair = std::optional<std::pair<PathIterConst, PathIterConst>>;

namespace PathUtils {
/*
    Input: Path, must have at least one space component and one data component.
    Output: Iterators to the first space component and the last data component of the path.
*/
auto path_range(std::filesystem::path const &path) -> OptPathIterConstPair {
    auto start = path.begin();
    auto end = path.end();
    end--; // now at data component
    if(*end=="/") // we only had '/'
        return {};
    if(*start=="/") // Sometimes we get a / in the start, remove it
        start++;
    if(start==end)
        return {};
    return std::make_pair(start, end);
}

auto data_name(PathIterConstPair const &iters) -> std::optional<std::string> {
    if(*iters.second=="")
        return {};    
    auto iter = iters.second;
    iter++;
    if(*iter=="")
        return *iters.second;
    return {};
}

auto space_name(PathIterConstPair const &iters) -> std::optional<std::string> {
    return *iters.first;
}

auto next(PathIterConstPair const &iters) -> PathIterConstPair {
    auto n = iters;
    n.first++;
    return n;
}

auto data_name(std::filesystem::path const &path) -> std::optional<std::string> {
    auto const name = path.filename();
    if(name=="")
        return {};
    return name;
}

auto remove_filename(std::filesystem::path const &path) -> std::filesystem::path {
    auto p = path;
    p.remove_filename();
    return p;
}
}

namespace Security {
    enum struct Type {
        Grab = 0,
        Insert
    };

    namespace Policy {
        using FunT = std::function<bool(Type const &sec, std::filesystem::path const &path)>;
        inline const FunT AlwaysAllow  = [](Type const &sec, std::filesystem::path const &path){ return true; };
        inline const FunT AlwaysReject = [](Type const &sec, std::filesystem::path const &path){ return false; };
    }

    struct Key {
        Key() = default;
        Key(Policy::FunT const &policy) : policy(policy) {}
        
        auto allow(Type const &type, std::filesystem::path const &path) const { return this->policy(type, path); }

        private:
            Policy::FunT policy = Policy::AlwaysAllow;
    };
}

class PathSpaceTE {
	struct concept_t {
		virtual ~concept_t() = default;
		
		virtual auto copy_()                                                                                          const -> std::unique_ptr<concept_t>     = 0;
		virtual auto size_()                                                                                          const -> int                            = 0;
		virtual auto insert_(std::filesystem::path const &path, DataType const &data)                                       -> bool                           = 0;
        virtual auto insert_(std::filesystem::path const &path, PathIterConstPair const &iters, DataType const &data)       -> bool                           = 0;
		virtual auto data_()                                                                                          const -> const std::optional<DataType>& = 0;
		virtual auto grab_(std::filesystem::path const &path)                                                               -> std::optional<PathSpaceTE>     = 0;
		virtual auto grab_(std::filesystem::path const &path, PathIterConstPair const &iters)                               -> std::optional<PathSpaceTE>     = 0;
		virtual auto grabBlock_(std::filesystem::path const &path)                                                          -> std::optional<PathSpaceTE>     = 0;
		virtual auto grabBlock_(std::filesystem::path const &path, PathIterConstPair const &iters)                          -> std::optional<PathSpaceTE>     = 0;
	};
public:
	PathSpaceTE() = default;
	template<typename T>
	PathSpaceTE(T x)                             : self(std::make_unique<model<T>>(std::move(x))) {}
	PathSpaceTE(PathSpaceTE const &rhs)          : self(rhs.self->copy_())                        {}
	PathSpaceTE(std::unique_ptr<concept_t> self) : self(std::move(self))                          {}

	auto operator=(PathSpaceTE const &rhs) -> PathSpaceTE& {return *this = PathSpaceTE(rhs);}
	auto operator=(PathSpaceTE&&) noexcept -> PathSpaceTE& = default;

	auto size()                                                                                                                    const -> int  { return this->self->size_(); }
	auto insert(std::filesystem::path const &path, DataType const &data)                                       -> bool { return this->self->insert_(path, data); }
	auto insert(std::filesystem::path const &path, PathIterConstPair const &iters, DataType const &data)       -> bool { return this->self->insert_(path, iters, data); }

    template<typename T>
    auto grab(std::filesystem::path const &path) -> std::optional<T> {
        if(auto val = this->self->grab_(path)) {
            if constexpr(std::is_same<T, PathSpaceTE>::value)
                return val.value();
            else if(val.value().data())
                return std::get<T>(val.value().data().value());
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
            else if(val.value().data())
                return std::get<T>(val.value().data().value());
        }
        return {};
    }

    auto grabBlock(std::filesystem::path const &path, PathIterConstPair const &iters) -> std::optional<PathSpaceTE> {
        return this->self->grab_(path, iters);
    }

	auto data() -> const std::optional<DataType>& { 
        return this->self->data_();
    }
private:
	template<typename T> 
	struct model final : concept_t {
		model(T x) : data(std::move(x)) {}

		auto copy_()                                                                             const          -> std::unique_ptr<concept_t>     override {return std::make_unique<model>(*this);}
		auto size_()                                                                             const          -> int                            override {return this->data.size();}
		auto insert_(std::filesystem::path const &path, DataType const &d)                                      -> bool                           override {return this->data.insert(path, d);}
		auto insert_(std::filesystem::path const &path, PathIterConstPair const &iters, DataType const &d)      -> bool                           override {return this->data.insert(path, iters, d);}
        auto data_()                                                                             const          -> const std::optional<DataType>& override {return this->data.data();}
		auto grab_(std::filesystem::path const &path)                                                           -> std::optional<PathSpaceTE>     override {return this->data.grab(path);}
		auto grab_(std::filesystem::path const &path, PathIterConstPair const &iters)                           -> std::optional<PathSpaceTE>     override {return this->data.grab(path, iters);}
		auto grabBlock_(std::filesystem::path const &path)                                                      -> std::optional<PathSpaceTE>     override {return this->data.grabBlock(path);}
		auto grabBlock_(std::filesystem::path const &path, PathIterConstPair const &iters)                      -> std::optional<PathSpaceTE>     override {return this->data.grabBlock(path, iters);}
		
		T data;
	};
	std::unique_ptr<concept_t> self;
};

struct PathSpace {
    PathSpace() = default;
    PathSpace(DataType const &data) : data_(data) {}
    PathSpace(PathSpace const &ps) : spaces(ps.spaces), data_(ps.data_) {}

    virtual auto insert(std::filesystem::path const &path, DataType const &data) -> bool {
        if(auto const iters = PathUtils::path_range(path))
            return this->insert(path, *iters, data);
        else if(auto name = PathUtils::data_name(path)) { // There is just one data space in the path, create and put the data in it
            std::lock_guard<std::shared_mutex> lock(this->mut); // write
            this->spaces.insert(std::make_pair(*name, PathSpace{data}));
            this->cv.notify_all();
            return true;
        }
        return false;
    };

    virtual auto insert(std::filesystem::path const &path, PathIterConstPair const &iters, DataType const &data) -> bool {
        std::lock_guard<std::shared_mutex> lock(this->mut); // write
        if(iters.first==path.end()) { // We are inserting the data into the previous space
            this->data_ = data;
            return true;
        }
        else if(auto space = this->findSpace(*iters.first)) {
            return space->insert(path, PathUtils::next(iters), data);
        } else if(auto const &name = PathUtils::space_name(iters)) {
            PathSpace newSpace;
            if(!newSpace.insert(path, PathUtils::next(iters), data))
                return false;
            this->spaces.insert(std::make_pair(*name, std::move(newSpace)));
            this->cv.notify_all();
            return true;
        }
        return false;
    };

    virtual auto grab(std::filesystem::path const &path) -> std::optional<PathSpaceTE> {
        if(auto const iters = PathUtils::path_range(path))
            return this->grab(path, *iters);
        else if(auto name = PathUtils::data_name(path)) { // There is just one data space in the path, grab it
            if(this->spaces.count(name.value())) {
                std::lock_guard<std::shared_mutex> lock(this->mut); // write
                return this->spaces.extract(name.value()).mapped();
            }
            return {};
        }
        return {};
    };

    virtual auto grab(std::filesystem::path const &path, PathIterConstPair const &iters) -> std::optional<PathSpaceTE> {
        std::lock_guard<std::shared_mutex> lock(this->mut); // write
        if(iters.first==iters.second) {
            if(auto name = PathUtils::data_name(path)) {
                if(this->spaces.count(name.value())>0)
                    return this->spaces.extract(name.value()).mapped();
                return {};
            }
        }
        else if(auto space = this->findSpace(*iters.first))
            return space->grab(path, PathUtils::next(iters));
        return {};
    };

    virtual auto grabBlock(std::filesystem::path const &path) -> std::optional<PathSpaceTE> {
        if(auto const iters = PathUtils::path_range(path))
            return this->grabBlock(path, *iters);
        else if(auto name = PathUtils::data_name(path)) { // There is just one data space in the path, grab it
            std::unique_lock<std::shared_mutex> lock(this->mut); // write
            while(!this->spaces.count(name.value()))
                this->cv.wait(lock);
            return this->spaces.extract(name.value()).mapped();
        }
        return {};
    };

    virtual auto grabBlock(std::filesystem::path const &path, PathIterConstPair const &iters) -> std::optional<PathSpaceTE> {
        std::unique_lock<std::shared_mutex> lock(this->mut); // write
        if(iters.first==iters.second) {
            if(auto name = PathUtils::data_name(path)) {
                while(!this->spaces.count(name.value()))
                    this->cv.wait(lock);
                return this->spaces.extract(name.value()).mapped();
            }
        }
        else if(auto space = this->findSpace(*iters.first))
            return space->grab(path, PathUtils::next(iters));
        return {};
    };

    virtual const std::optional<DataType>& data() const {
        return this->data_;
    }

    virtual auto size() const -> int {
        std::shared_lock<std::shared_mutex> lock(this->mut); // read
        return this->spaces.size() + (this->data_.has_value() ? 1 : 0);
    }

private:
    auto findSpace(std::string const &name) -> PathSpaceTE* {
        auto const range = this->spaces.equal_range(name);
        for (auto it = range.first; it != range.second; ++it) 
            if(it->first==name)
                return &it->second;
        return nullptr;
    }

    mutable std::shared_mutex mut;
    mutable std::condition_variable_any cv;
    std::unordered_multimap<std::string, PathSpaceTE> spaces;
    std::optional<DataType> data_;
};

template<typename T>
struct View {
    View(T const &space, Security::Key const &key) : space(space), key(key) {};
    
    auto insert(std::filesystem::path const &path, DataType const &data) -> void {
        if(this->key.allow(Security::Type::Insert, path))
            this->space->insert(path, data);
    };

    template <typename U>
    auto grab(std::filesystem::path const &path) -> std::optional<U> { 
        if(this->key.allow(Security::Type::Grab, path)) {
            if(auto const val = this->space->template grab<U>(path))
                return val.value();
        }
        return {};
    };

    template <typename U>
    auto grabBlock(std::filesystem::path const &path) -> std::optional<U> {
        if(this->key.allow(Security::Type::Grab, path)) {
            if(auto const val = this->space->template grabBlock<U>(path))
                return val.value();
        }
        return {};
    };

    auto size() const -> int {
        return this->space->size();
    }

private:
    T space;
    Security::Key key;
};

}