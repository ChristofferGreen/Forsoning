#pragma once

#include <condition_variable>
#include <deque>
#include <functional>
#include <filesystem>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <variant>
#include <experimental/coroutine>

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

template<typename T>
struct Coro {
    struct promise_type;
    using handle_type = std::experimental::coroutine_handle<promise_type>;

    Coro(handle_type h): coro(h) {}
    handle_type coro;

    ~Coro() {
        if ( coro ) coro. destroy();
    }
    Coro(const Coro&) = delete;
    Coro& operator= (const Coro&) = delete;
    Coro(Coro&& oth) noexcept : coro(oth.coro){
        oth.coro = nullptr;
    }
    Coro& operator = (Coro&& oth) noexcept {
        coro = oth.coro;
        oth.coro = nullptr;
        return *this;
    }
    T getValue() {
        return coro.promise().current_value;
    }
    bool next() {
        coro.resume();
        return not coro.done();
    }
    struct promise_type {
        promise_type() = default;
        ~promise_type() = default;

        auto initial_suspend() {
            return std::experimental::suspend_always{};
        }
        auto final_suspend() noexcept {
            return std::experimental::suspend_always{};
        }
        auto get_return_object() {
            return Coro {handle_type::from_promise(*this)};
        }
        auto return_void() {
            return std::experimental::suspend_never{};
        }
        auto yield_value(const T value) {
            current_value = value;
            return std::experimental::suspend_always{};
        }
        void unhandled_exception() {
            std::exit(1);
        }
        T current_value;
    };
};

class PathSpaceTE {
	struct concept_t {
		virtual ~concept_t() = default;
		
		virtual auto copy_()                                                                                          const -> std::unique_ptr<concept_t> = 0;
		virtual auto nbrSpaces_()                                                                                     const -> int                        = 0;
		virtual auto insert_(std::filesystem::path const &path, DataType const &data)                                       -> bool                       = 0;
		virtual auto insert_(std::filesystem::path const &path, std::function<Coro<DataType>()> const &fun)                 -> bool                       = 0;
        virtual auto insert_(std::filesystem::path const &path, PathIterConstPair const &iters, DataType const &data)       -> bool                       = 0;
		virtual auto popFrontData_()                                                                                        -> std::optional<DataType>    = 0;
		virtual auto grab_(std::filesystem::path const &path)                                                               -> std::optional<PathSpaceTE> = 0;
		virtual auto grab_(std::filesystem::path const &path, PathIterConstPair const &iters)                               -> std::optional<PathSpaceTE> = 0;
		virtual auto grabBlock_(std::filesystem::path const &path)                                                          -> std::optional<PathSpaceTE> = 0;
		virtual auto grabBlock_(std::filesystem::path const &path, PathIterConstPair const &iters)                          -> std::optional<PathSpaceTE> = 0;
	};
public:
	PathSpaceTE() = default;
	template<typename T>
	PathSpaceTE(T x)                             : self(std::make_unique<model<T>>(std::move(x))) {}
	PathSpaceTE(PathSpaceTE const &rhs)          : self(rhs.self->copy_())                        {}
	PathSpaceTE(std::unique_ptr<concept_t> self) : self(std::move(self))                          {}

	auto operator=(PathSpaceTE const &rhs) -> PathSpaceTE& {return *this = PathSpaceTE(rhs);}
	auto operator=(PathSpaceTE&&) noexcept -> PathSpaceTE& = default;

	auto nbrSpaces()                                                                                     const -> int  { return this->self->nbrSpaces_(); }
	auto insert(std::filesystem::path const &path, DataType const &data)                                       -> bool { return this->self->insert_(path, data); }
	auto insert(std::filesystem::path const &path, std::function<Coro<DataType>()> const &fun)                 -> bool { return this->self->insert_(path, fun); }
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
    }
private:
	template<typename T> 
	struct model final : concept_t {
		model(T x) : data(std::move(x)) {}

		auto copy_()                                                                                      const -> std::unique_ptr<concept_t>     override {return std::make_unique<model>(*this);}
		auto nbrSpaces_()                                                                                 const -> int                            override {return this->data.nbrSpaces();}
		auto insert_(std::filesystem::path const &path, DataType const &d)                                      -> bool                           override {return this->data.insert(path, d);}
		auto insert_(std::filesystem::path const &path, std::function<Coro<DataType>()> const &fun)             -> bool                           override {return this->data.insert(path, fun);}
		auto insert_(std::filesystem::path const &path, PathIterConstPair const &iters, DataType const &d)      -> bool                           override {return this->data.insert(path, iters, d);}
        auto popFrontData_()                                                                                    -> std::optional<DataType>        override {return this->data.popFrontData();}
		auto grab_(std::filesystem::path const &path)                                                           -> std::optional<PathSpaceTE>     override {return this->data.grab(path);}
		auto grab_(std::filesystem::path const &path, PathIterConstPair const &iters)                           -> std::optional<PathSpaceTE>     override {return this->data.grab(path, iters);}
		auto grabBlock_(std::filesystem::path const &path)                                                      -> std::optional<PathSpaceTE>     override {return this->data.grabBlock(path);}
		auto grabBlock_(std::filesystem::path const &path, PathIterConstPair const &iters)                      -> std::optional<PathSpaceTE>     override {return this->data.grabBlock(path, iters);}
		
		T data;
	};
	std::unique_ptr<concept_t> self;
};

struct Aegis {
    Aegis() = default;
    Aegis(Aegis const &other) : spaces(other.spaces), data(other.data) {}
    Aegis(DataType const &d) {this->data.push_back(d);}

    auto insert(std::pair<std::string, PathSpaceTE> const &p) {
        std::lock_guard<std::shared_mutex> lock(this->spacesMut); // write
        this->spaces.insert(p);
        this->cv.notify_all();
    }

    auto count(std::string const &name) const {
        std::shared_lock<std::shared_mutex> lock(this->spacesMut); // read
        return this->spaces.count(name);
    }

    auto nbrSpaces() const {
        std::shared_lock<std::shared_mutex> lock(this->spacesMut); // read
        return this->spaces.size();
    }

    auto extract(std::string const &name) -> std::optional<PathSpaceTE> {
        std::lock_guard<std::shared_mutex> lock(this->spacesMut); // write
        if(this->spaces.count(name))
            return this->spaces.extract(name).mapped();
        return {};
    }

    auto waitExtract(std::string const &name) {
        std::unique_lock<std::shared_mutex> lock(this->spacesMut); // write
        while(!this->spaces.count(name))
            this->cv.wait(lock);
        return this->spaces.extract(name).mapped();
    }

    auto grabRecurse(std::filesystem::path const &path, PathIterConstPair const &iters, std::string const &name) -> std::optional<PathSpaceTE> {
        if(auto space = this->findSpace(*iters.first))
            return space->grab(path, PathUtils::next(iters));
        return {};
   }

    auto insertRecurse(std::filesystem::path const &path, PathIterConstPair const &iters, DataType const &data) -> bool {
        if(auto space = this->findSpace(*iters.first)) {
            return space->insert(path, PathUtils::next(iters), data);
            std::unique_lock<std::shared_mutex> lock(this->spacesMut); // write
        }
        return false;
    }

    // Data methods
    auto pushBackData(DataType const &d) {
        std::unique_lock<std::shared_mutex> lock(this->dataMut); // write
        /*if (std::holds_alternative<ExecutionType>(d)) {
            std::get<ExecutionType>(d)();
        }*/
        this->data.push_back(d);
    }

    auto popFrontData() -> std::optional<DataType> {
        std::unique_lock<std::shared_mutex> lock(this->dataMut); // write
        if(this->data.size()==0)
            return std::nullopt;
        auto const val = this->data.front();
        this->data.pop_front();
        return val;
    }

    auto dataHasValue() const {
        std::shared_lock<std::shared_mutex> lock(this->dataMut); // read
        return this->data.size()>0;
    }
private:
    auto findSpace(std::string const &name) -> PathSpaceTE* {
        std::shared_lock<std::shared_mutex> lock(this->spacesMut); // read
        auto const range = this->spaces.equal_range(name);
        for (auto it = range.first; it != range.second; ++it) 
            if(it->first==name)
                return &it->second;
        return nullptr;
    }
protected:
    std::unordered_multimap<std::string, PathSpaceTE> spaces;
    std::deque<DataType> data;
    mutable std::shared_mutex spacesMut, dataMut;
    mutable std::condition_variable_any cv;
};

struct TaskExecutor {

};

struct PathSpace {
    PathSpace() = default;
    PathSpace(DataType const &data) {this->aegis.pushBackData(data);}
    PathSpace(PathSpace const &ps) : aegis(ps.aegis) {}

    virtual auto insert(std::filesystem::path const &path, std::function<Coro<DataType>()> const &fun) -> bool {
        return false;
    }

    virtual auto insert(std::filesystem::path const &path, DataType const &data) -> bool {
        if(auto const iters = PathUtils::path_range(path))
            return this->insert(path, *iters, data);
        else if(auto name = PathUtils::data_name(path)) { // There is just one data space in the path, create and put the data in it
            if(this->aegis.count(name.value())>0) {
                //this->aegis.pushBackData();
            } else {
                this->aegis.insert(std::make_pair(*name, PathSpace{data}));
            }
            return true;
        }
        return false;
    };

    virtual auto insert(std::filesystem::path const &path, PathIterConstPair const &iters, DataType const &data) -> bool {
        if(iters.first==path.end()) {
            this->aegis.pushBackData(data);
            return true;
        }
        else if(this->aegis.insertRecurse(path, PathUtils::next(iters), data)) {}
        else if(auto const &name = PathUtils::space_name(iters)) {
            PathSpace newSpace;
            if(!newSpace.insert(path, PathUtils::next(iters), data))
                return false;
            this->aegis.insert(std::make_pair(*name, std::move(newSpace)));
            return true;
        }
        return false;
    };

    virtual auto grab(std::filesystem::path const &path) -> std::optional<PathSpaceTE> {
        if(auto const iters = PathUtils::path_range(path))
            return this->grab(path, *iters);
        else if(auto name = PathUtils::data_name(path)) { // There is just one data space in the path, grab it
            if(this->aegis.count(name.value()))
                return this->aegis.extract(name.value());
            return {};
        }
        return {};
    };

    virtual auto grab(std::filesystem::path const &path, PathIterConstPair const &iters) -> std::optional<PathSpaceTE> {
        if(iters.first==iters.second) {
            if(auto name = PathUtils::data_name(path)) {
                if(this->aegis.count(name.value())>0)
                    return this->aegis.extract(name.value());
                return {};
            }
        }
        else
            return this->aegis.grabRecurse(path, iters, *iters.first);
        return {};
    };

    virtual auto grabBlock(std::filesystem::path const &path) -> std::optional<PathSpaceTE> {
        if(auto const iters = PathUtils::path_range(path))
            return this->grabBlock(path, *iters);
        else if(auto name = PathUtils::data_name(path)) // There is just one data space in the path, grab it
            return this->aegis.waitExtract(name.value());
        return {};
    };

    virtual auto grabBlock(std::filesystem::path const &path, PathIterConstPair const &iters) -> std::optional<PathSpaceTE> {
        if(iters.first==iters.second) {
            if(auto name = PathUtils::data_name(path))
                return this->aegis.waitExtract(name.value());
        }
        else
            return this->aegis.grabRecurse(path, iters, *iters.first);
        return {};
    };

    virtual std::optional<DataType> popFrontData() {
        return this->aegis.popFrontData();
    }

    virtual auto nbrSpaces() const -> int {
        return this->aegis.nbrSpaces();
    }
private:
    Aegis aegis;
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

    auto nbrSpaces() const -> int {
        return this->space->nbrSpaces();
    }

private:
    T space;
    Security::Key key;
};

}