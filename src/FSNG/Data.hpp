#pragma once
#include <cassert>
#include <concepts>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <memory>

#include "InReference.hpp"
#include "Converters.hpp"
#include "utils.hpp"

namespace FSNG {
struct PathSpaceTE;
struct Coroutine;
struct CoroutineVoid;

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
concept ReturnsCoroutine = requires(T t) {
    {t()} -> std::same_as<Coroutine>;
};

template<typename T>
concept ReturnsCoroutineVoid = requires(T t) {
    {t()} -> std::same_as<CoroutineVoid>;
};

template<typename T>
concept NotTriviallyCopyable = !std::is_trivially_copyable<T>::value;

template<typename T>
concept HasJSONConversionNotTriviallyCopyableNoByteVector = HasJSONConversion<T> && NotTriviallyCopyable<T> && !HasByteVectorConversion<T>;

struct Data {
    Data() = default;
    Data(bool                    const &b)    : data(b), isTriviallyCopyable_(false) {this->eat(b);}
    Data(signed char             const &c)    : data(c), isTriviallyCopyable_(false) {this->eat(c);}
    Data(unsigned char           const &c)    : data(c), isTriviallyCopyable_(false) {this->eat(c);}
    Data(wchar_t                 const &c)    : data(c), isTriviallyCopyable_(false) {this->eat(c);}
    Data(short                   const &s)    : data(s), isTriviallyCopyable_(false) {this->eat(s);}
    Data(unsigned short          const &s)    : data(s), isTriviallyCopyable_(false) {this->eat(s);}
    Data(int                     const &i)    : data(i), isTriviallyCopyable_(true) {this->eat(i);}
    Data(unsigned int            const &i)    : data(i), isTriviallyCopyable_(false) {this->eat(i);}
    Data(long                    const &l)    : data(l), isTriviallyCopyable_(false) {this->eat(l);}
    Data(unsigned long           const &l)    : data(l), isTriviallyCopyable_(false) {this->eat(l);}
    Data(long long               const &l)    : data(l), isTriviallyCopyable_(false) {this->eat(l);}
    Data(unsigned long long      const &l)    : data(l), isTriviallyCopyable_(false) {this->eat(l);}
    Data(double                  const &d)    : data(d), isTriviallyCopyable_(false) {this->eat(d);}
    Data(long double             const &d)    : data(d), isTriviallyCopyable_(false) {this->eat(d);}
    Data(char const *            const s)     : data(s), isTriviallyCopyable_(false) {this->eat(s);}
    Data(std::string             const &s)    : data(s) {this->eat(s);}
    Data(std::unique_ptr<PathSpaceTE> &&up)   : data(std::move(up)) {this->eat(up);}
    Data(PathSpaceTE             const &pste) : data(std::make_unique<PathSpaceTE>(pste)) {this->eat(pste);}
    Data(PathSpaceTE                   *pste) : data(pste) {this->eat(pste);}
    Data(ReturnsCoroutine auto   const &in) {
        data = std::make_unique<std::function<Coroutine()>>(in);
        this->eat(in);
    }
    Data(ReturnsCoroutineVoid auto   const &in) {
        data = std::make_unique<std::function<CoroutineVoid()>>(in);
        this->eat(in);
    }
    Data(TriviallyCopyableButNotInvocable auto const &in) {
        using InT = decltype(in);
        this->data = InReference{&in, sizeof(InT), &typeid(in)};
        if(!Converters::toJSONConverters.contains(&typeid(in))) {
            Converters::toJSONConverters[&typeid(in)] = [](std::byte const *data, int const size){
                nlohmann::json out;
                to_json(out, reinterpret_cast<InT>(*data));
                return out;
            };
        }
        this->eat(in);
    }
    Data(HasByteVectorConversion auto const &in) {
        using InT = decltype(in);
        using InTRR = typename std::remove_reference<InT>::type;
        using InTRRRC = typename std::remove_const<InTRR>::type;
        this->data = InReference{&in, sizeof(InT), &typeid(in)};
        if(!Converters::toJSONConverters.contains(&typeid(in))) {
            Converters::toJSONConverters[&typeid(in)] = [](std::byte const *data, int const size) {
                nlohmann::json out;
                InTRRRC value;
                from_bytevec(data, value);
                to_json(out, value);
                return out;
            };
        }
        if(!Converters::toByteArrayConverters.contains(&typeid(in))) {
            Converters::toByteArrayConverters[&typeid(in)] = [](std::vector<std::byte> &vec, void const *obj) {
                to_bytevec(vec, *static_cast<InTRR*>(obj));
            };
        }
        if(!Converters::fromByteArrayConverters.contains(&typeid(in))) {
            Converters::fromByteArrayConverters[&typeid(in)] = [](std::byte const *fromBytes, void *toObj) {
                try {
                    from_bytevec(fromBytes, *static_cast<InTRRRC*>(toObj));
                } catch(std::exception const &e) {
                    return false;
                }
                return true;
            };
        }
        this->eat(in);
    }
    Data(HasJSONConversionNotTriviallyCopyableNoByteVector auto const &in) {
        using InT = decltype(in);
        using InTRR = typename std::remove_reference<InT>::type;
        using InTRRRC = typename std::remove_const<InTRR>::type;
        this->data = InReference{&in, sizeof(InT), &typeid(in)};
        if(!Converters::toJSONConverters.contains(&typeid(in))) {
            Converters::toJSONConverters[&typeid(in)] = [](std::byte const *data, int const size) {
                return nlohmann::json::from_bson(std::vector<std::byte>(data, data+size));
            };
        }
        if(!Converters::fromJSONConverters.contains(&typeid(in))) {
            Converters::fromJSONConverters[&typeid(in)] = [](std::byte const *fromBytes, int const size, void *toObject) {
                try {
                    auto json = nlohmann::json::from_bson(std::vector<std::byte>(fromBytes, fromBytes+size));
                    *static_cast<InTRRRC*>(toObject) = json.get<InTRRRC>();
                } catch(std::exception const &e) {
                    return false;
                }
                return true;
            };
        }
        if(!Converters::toByteArrayConverters.contains(&typeid(in))) {
            Converters::toByteArrayConverters[&typeid(in)] = [](std::vector<std::byte> &vec, void const *obj) {
                nlohmann::json out;
                to_json(out, *static_cast<InTRR*>(obj));
                std::vector<std::uint8_t> v_bson = nlohmann::json::to_bson(out);
                copy_byte_back_insert(v_bson.data(), v_bson.size(), vec);
            };
        }
        this->eat(in);
    }

    template<typename T>
    auto is() const {
        return std::holds_alternative<T>(this->data);
    }

    template<typename T>
    auto& as() const {
        return std::get<T>(this->data);
    }

    bool isTriviallyCopyable() const {
        return this->isTriviallyCopyable_;
    }

    template<typename T>
    auto eat(T const &in) -> void {
        this->ptr = &in;
        this->size = sizeof(in);
        this->info = &typeid(in);
    }

    void const *ptr = nullptr;
    int size = 0;
    std::type_info const *info = nullptr;
private:
    std::variant<bool,
                 signed char,
                 unsigned char,
                 wchar_t,
                 short,
                 unsigned short,
                 int,
                 unsigned int,
                 long,
                 unsigned long,
                 long long,
                 unsigned long long,
                 double,
                 long double,
                 char const *,
                 std::string,
                 PathSpaceTE*,
                 std::unique_ptr<PathSpaceTE>,
                 std::unique_ptr<std::function<Coroutine()>>,
                 std::unique_ptr<std::function<CoroutineVoid()>>,
                 InReference,
                 std::vector<std::byte>> data;
    bool isTriviallyCopyable_ = false;
};
}