#pragma once
#include <cassert>
#include <concepts>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <memory>

#include "InReference.hpp"

namespace FSNG {

template<typename T>
concept TriviallyCopyableButNotInvocable = std::is_trivially_copyable<T>::value && !std::is_invocable<T>::value;

template<typename T>
concept HasByteVectorConversion  = requires(T t, std::vector<std::byte> &vec) {
    to_bytevec(vec, t);
};

struct PathSpaceTE;
struct Coroutine;
struct Data {
    Data() = default;
    Data(int i) : data(i) {}
    Data(char const *s) : data(std::string(s)) {}
    Data(std::string const &s) : data(s) {}
    Data(std::unique_ptr<PathSpaceTE> &&up) : data(std::move(up)) {}
    Data(PathSpaceTE const &pste) : data(std::make_unique<PathSpaceTE>(pste)) {}
    Data(std::invocable auto const &in) {
        data = std::make_unique<std::function<Coroutine()>>(in);
    }
    Data(TriviallyCopyableButNotInvocable auto const &in) {
        using InT = decltype(in);
        this->data = InReference{&in, sizeof(InT), &typeid(in)};
        if(InReference::toJSONConverters.count(&typeid(in))==0) {
            InReference::toJSONConverters[&typeid(in)] = [](std::byte const *data){
                nlohmann::json out;
                to_json(out, reinterpret_cast<InT>(*data));
                return out;
            };
        }
    }
    Data(HasByteVectorConversion auto const &in) {
        using InT = decltype(in);
        using InTRR = typename std::remove_reference<InT>::type;
        this->data = InReference{&in, sizeof(InT), &typeid(in)};
        if(InReference::toJSONConverters.count(&typeid(in))==0) {
            InReference::toJSONConverters[&typeid(in)] = [](std::byte const *data){
                nlohmann::json out;
                to_json(out, reinterpret_cast<InT>(*data));
                return out;
            };
        }
        if(InReference::toByteArrayconverters.count(&typeid(in))==0) {
            InReference::toByteArrayconverters[&typeid(in)] = [](std::vector<std::byte> &vec, void *obj){
                nlohmann::json out;
                to_bytevec(vec, *static_cast<InTRR*>(obj));
                return out;
            };
        }
    }

    template<typename T>
    auto is() const {
        return std::holds_alternative<T>(this->data);
    }

    template<typename T>
    auto& as() const {
        return std::get<T>(this->data);
    }

    auto isTriviallyCopyable() const {
        if(std::holds_alternative<int>(this->data) ||
           std::holds_alternative<double>(this->data)) {
               return true;
        }
        else if(std::holds_alternative<InReference>(this->data)) {
            return this->as<InReference>().isTriviallyCopyable;
        };
        return false;
    }

private:
    std::variant<int,
                 double,
                 std::string,
                 std::unique_ptr<PathSpaceTE>,
                 std::unique_ptr<std::function<Coroutine()>>,
                 InReference> data;
};
}