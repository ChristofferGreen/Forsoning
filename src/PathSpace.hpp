#pragma once
#include "FSNG/Coro.hpp"
#include "FSNG/Data.hpp"
#include "FSNG/Path.hpp"
#include "FSNG/PathSpaceTE.hpp"
#include "FSNG/Security.hpp"

#include "nlohmann/json.hpp"

#include <deque>
#include <unordered_map>
#include <string>
#include <variant>

namespace FSNG {
struct PathSpace {
    PathSpace() = default;
    //PathSpace(DataType const &data) {this->aegis.pushBackData(data);}
    //PathSpace(PathSpace const &ps) : aegis(ps.aegis) {}

    /*virtual auto insert(std::filesystem::path const &path, std::function<Coro<DataType>()> const &fun) -> bool {
        return false;
    }*/

    virtual auto insert(Path const &path, Data const &data) -> bool {
        /*if(auto const iters = PathUtils::path_range(path))
            return this->insert(path, *iters, data);
        else if(auto name = PathUtils::data_name(path)) { // There is just one data space in the path, create and put the data in it
            if(this->aegis.count(name.value())>0) {
                //this->aegis.pushBackData();
            } else {
                this->aegis.insert(std::make_pair(*name, PathSpace{data}));
            }
            return true;
        }
        return false;*/
        if(auto range = path.range())
            return this->insert(range.value(), data);
        return false;
    };

    virtual auto insert(Path::Range const &range, Data const &data) -> bool {
        if(range.isAtData())
            return this->insert(range.dataName(), data);
        return false;
    }

    virtual auto insert(std::string const &dataName, Data const &data) -> bool {
        if(data.is<int>())
            this->insertT<int>(dataName, data);
        else if(data.is<double>())
            this->insertT<double>(dataName, data);
        else if(data.is<std::string>())
            this->insertT<std::string>(dataName, data);
        else if(data.is<std::unique_ptr<PathSpaceTE>>())
            this->insertSpaceT(dataName, data);
        else
            return false;
        return true;
    }

    virtual auto toJSON() const -> nlohmann::json {
        nlohmann::json json;
        for(auto const &p : this->data) {
            nlohmann::json array;
            if(std::holds_alternative<std::deque<int>>(this->data.at(p.first)))
                for(auto &v : std::get<std::deque<int>>(p.second))
                    array.push_back(v);
            else if(std::holds_alternative<std::deque<double>>(this->data.at(p.first)))
                for(auto &v : std::get<std::deque<double>>(p.second))
                    array.push_back(v);
            else if(std::holds_alternative<std::deque<std::string>>(this->data.at(p.first)))
                for(auto &v : std::get<std::deque<std::string>>(p.second))
                    array.push_back(v);
            /*else if(std::holds_alternative<std::deque<std::unique_ptr<PathSpaceTE>>>(this->data.at(p.first)))
                for(auto &v : std::get<std::deque<std::unique_ptr<PathSpaceTE>>>(p.second))
                    array.push_back(v.toJSON());*/
            json[p.first] = std::move(array);
        }
        return json;
    }

    /*virtual auto insert(std::filesystem::path const &path, PathIterConstPair const &iters, DataType const &data) -> bool {
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
    }*/
private:
    template<typename T>
    auto insertT(std::string const &dataName, Data const &data) -> void {
        if(this->data.count(dataName))
            std::get<std::deque<T>>(this->data[dataName]).push_back(data.as<T>());
        else
            this->data.insert(std::make_pair(dataName, std::deque<T>{data.as<T>()}));
    }
    auto insertSpaceT(std::string const &dataName, Data const &data) -> void {
    }

    using varT = std::variant<std::deque<int>, std::deque<double>, std::deque<std::string>, std::deque<PathSpaceTE>>;
    std::unordered_map<std::string, varT> data;
};
}