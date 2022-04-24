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
concept HasByteVectorConversion = requires(typename std::remove_const<T>::type t, std::vector<std::byte> &vec) {
    to_bytevec(vec, t);
    from_bytevec(vec.data(), t);
};

template<typename T>
concept HasJSONConversion = requires(typename std::remove_const<T>::type t, nlohmann::json &json) {
    to_json(json, t);
};

template<typename T>
concept NotTriviallyCopyable = !std::is_trivially_copyable<T>::value;

template<typename T>
concept HasJSONConversionNotTriviallyCopyableNoByteVector = HasJSONConversion<T> && NotTriviallyCopyable<T> && !HasByteVectorConversion<T>;

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
        this->data = InReference{&in, sizeof(InT), &typeid(in), true};
        if(!InReference::toJSONConverters.contains(&typeid(in))) {
            InReference::toJSONConverters[&typeid(in)] = [](std::byte const *data, int const size){
                nlohmann::json out;
                to_json(out, reinterpret_cast<InT>(*data));
                return out;
            };
        }

    }
    Data(HasByteVectorConversion auto const &in) {
        using InT = decltype(in);
        using InTRR = typename std::remove_reference<InT>::type;
        using InTRRRC = typename std::remove_const<InTRR>::type;
        this->data = InReference{&in, sizeof(InT), &typeid(in)};
        if(!InReference::toJSONConverters.contains(&typeid(in))) {
            InReference::toJSONConverters[&typeid(in)] = [](std::byte const *data, int const size) {
                nlohmann::json out;
                InTRRRC value;
                from_bytevec(data, value);
                to_json(out, value);
                return out;
            };
        }
        if(!InReference::toByteArrayConverters.contains(&typeid(in))) {
            InReference::toByteArrayConverters[&typeid(in)] = [](std::vector<std::byte> &vec, void const *obj) {
                to_bytevec(vec, *static_cast<InTRR*>(obj));
            };
        }
        if(!InReference::fromByteArrayConverters.contains(&typeid(in))) {
            InReference::fromByteArrayConverters[&typeid(in)] = [](std::byte const *vec, void *obj) {
                from_bytevec(vec, *static_cast<typename std::remove_const<InTRR>::type*>(obj));
            };
        }
    }
    Data(HasJSONConversionNotTriviallyCopyableNoByteVector auto const &in) {
        using InT = decltype(in);
        using InTRR = typename std::remove_reference<InT>::type;
        this->data = InReference{&in, sizeof(InT), &typeid(in)};
        if(!InReference::toJSONConverters.contains(&typeid(in))) {
            InReference::toJSONConverters[&typeid(in)] = [](std::byte const *data, int const size){
                return nlohmann::json::from_bson(std::vector<std::byte>(data, data+size));
            };
        }
        if(!InReference::toByteArrayConverters.contains(&typeid(in))) {
            InReference::toByteArrayConverters[&typeid(in)] = [](std::vector<std::byte> &vec, void const *obj) {
                nlohmann::json out;
                to_json(out, *static_cast<InTRR*>(obj));
                std::vector<std::uint8_t> v_bson = nlohmann::json::to_bson(out);
                std::copy(static_cast<std::byte const * const>(static_cast<void const * const>(v_bson.data())),
                          static_cast<std::byte const * const>(static_cast<void const * const>(v_bson.data())) + v_bson.size(),
                          std::back_inserter(vec));
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