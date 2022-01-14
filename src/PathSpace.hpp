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
using PathIterConst        = std::filesystem::path::const_iterator;
using PathIterConstPair    = std::pair<PathIterConst, PathIterConst>;
using OptPathIterConstPair = std::optional<std::pair<PathIterConst, PathIterConst>>;
namespace PathUtils {
/*
    Input: Path, must have at least one space component and one data component.
    Output: Iterators to the first space component and the last space component of the path.
*/
auto path_range(std::filesystem::path const &path) -> OptPathIterConstPair {
    auto start = path.begin();
    auto end = path.end();
    end--; // now at file
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
        
        auto allow(Type const &type, std::filesystem::path const &path) { return this->policy(type, path); }

        private:
            Policy::FunT policy = Policy::AlwaysAllow;
    };
}

using DataType = std::variant<int, std::string>;

class PathSpaceTE {
	struct concept_t {
		virtual ~concept_t() = default;
		
		virtual auto copy_()                                                                                    const -> std::unique_ptr<concept_t>     = 0;
		virtual auto size_()                                                                                    const -> int                            = 0;
		virtual auto insert_(std::filesystem::path const &path, DataType const &data, Security::Key const &key)       -> bool                           = 0;
        virtual auto insert_(PathIterConstPair const &iters, DataType const &data, Security::Key const &key)          -> bool                           = 0;
		virtual auto data_()                                                                                    const -> const std::optional<DataType>& = 0;
		virtual auto grab_(std::filesystem::path const &path, Security::Key const &key)                               -> std::optional<PathSpaceTE>     = 0;
		virtual auto grabBlock_(std::filesystem::path const &path, Security::Key const &key)                          -> std::optional<PathSpaceTE>     = 0;
	};
public:
	PathSpaceTE() = default;
	template<typename T>
	PathSpaceTE(T x)                             : self(std::make_unique<model<T>>(std::move(x))) {}
	PathSpaceTE(PathSpaceTE const &rhs)          : self(rhs.self->copy_())                        {}
	PathSpaceTE(std::unique_ptr<concept_t> self) : self(std::move(self))                          {}

	auto operator=(PathSpaceTE const &rhs) -> PathSpaceTE& {return *this = PathSpaceTE(rhs);}
	auto operator=(PathSpaceTE&&) noexcept -> PathSpaceTE& = default;

	auto size()                                                                                   const -> int  { return this->self->size_(); }
	auto insert(std::filesystem::path const &path, DataType const &data, Security::Key const &key)      -> bool { return this->self->insert_(path, data, key); }
	auto insert(PathIterConstPair const &iters, DataType const &data, Security::Key const &key)         -> bool { return this->self->insert_(iters, data, key); }

    template<typename T>
    auto grab(std::filesystem::path const &path, Security::Key const &key) -> std::optional<T> {
        if(auto val = this->self->grab_(path, key)) {
            if constexpr(std::is_same<T, PathSpaceTE>::value)
                return val.value();
            else if(val.value().data())
                return std::get<T>(val.value().data().value());
        }
        return {};
    }

    template<typename T>
	auto grabBlock(std::filesystem::path const &path, Security::Key const &key) -> std::optional<T> { 
        return {};
    }

	auto data() -> const std::optional<DataType>& { 
        return this->self->data_();
    }
private:
	template<typename T> 
	struct model final : concept_t {
		model(T x) : data(std::move(x)) {}

		auto copy_()                                                                             const -> std::unique_ptr<concept_t>     override {return std::make_unique<model>(*this);}
		auto size_()                                                                             const -> int                            override {return this->data.size();}
		auto insert_(std::filesystem::path const &path, DataType const &d, Security::Key const &key)   -> bool                           override {return this->data.insert(path, d, key);}
		auto insert_(PathIterConstPair const &iters, DataType const &d, Security::Key const &key)      -> bool                           override {return this->data.insert(iters, d, key);}
        auto data_()                                                                             const -> const std::optional<DataType>& override {return this->data.data();}
		auto grab_(std::filesystem::path const &path, Security::Key const &key)                        -> std::optional<PathSpaceTE>     override {return this->data.grab(path, key);}
		auto grabBlock_(std::filesystem::path const &path, Security::Key const &key)                   -> std::optional<PathSpaceTE>     override {return this->data.grabBlock(path, key);}
		
		T data;
	};
	std::unique_ptr<concept_t> self;
};

struct PathSpace {
    PathSpace() = default;
    PathSpace(DataType const &data) : data_(data) {}
    PathSpace(PathSpace const &ps) : spaces(ps.spaces), data_(ps.data_) {}

    virtual auto insert(std::filesystem::path const &path, DataType const &data, Security::Key const &key) -> bool {
        if(auto const iters = PathUtils::path_range(path))
            return this->insert(*iters, data, key);
        else if(auto name = PathUtils::data_name(path)) { // There is just one data space in the path, create and put the data in it
            this->spaces.insert(std::make_pair(*name, PathSpace{data}));
            return true;
        }
        return false;
    };

    virtual auto insert(PathIterConstPair const &iters, DataType const &data, Security::Key const &key) -> bool {
        auto next = iters;
        next.first++;
        if(iters.first==iters.second) {
            this->data_ = data;
            return true;
        }
        else if(auto space = this->findSpace(*iters.first)) {
            return space->insert(next, data, key);
        } else if(auto const &name = PathUtils::data_name(iters)) {
            PathSpace newSpace;
            if(!newSpace.insert(next, data, key))
                return false;
            this->spaces.insert(std::make_pair(*name, std::move(newSpace)));
            return true;
        }
        return false;
    };

    virtual auto grab(std::filesystem::path const &path, Security::Key const &key) -> std::optional<PathSpaceTE> {
        return {};
    };

    virtual auto grabBlock(std::filesystem::path const &path, Security::Key const &key) -> std::optional<PathSpaceTE> {
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
    /*auto createNonExistingSpaces(std::filesystem::path::const_iterator const &current, std::filesystem::path::const_iterator const &end) -> void {
        auto next = current;
        next++;
        if(auto existingSpace = this->findSpace(*current)) {
            existingSpace->createNonExistingSpaces(next, end);
            return;
        }
        PathSpace newSpace;
        if(current!=end) {
            newSpace.createNonExistingSpaces(next, end);
        }
        this->spaces.insert(std::make_pair(*current, newSpace));
    }*/

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

}