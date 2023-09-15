#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <type_traits>

#include "Converters.hpp"
#include "utils.hpp"

#include "nlohmann/json.hpp"

namespace FSNG {

template<typename T>
concept has_byte_vector_conversion = requires(typename std::remove_const<T>::type t, std::vector<std::byte> &vec) {
    to_bytevec(vec, t);
    from_bytevec(vec.data(), t);
};

template<typename T>
concept has_json_conversion = requires(typename std::remove_const<T>::type t, nlohmann::json &json) {
    to_json(json, t);
};

template <typename T>
concept IsPathSpace2 = requires { typename T::IsPathSpace2Type; };

struct TypeInfo {
    // std::string
    template<typename T>
    requires std::same_as<T, std::string>
    static consteval auto Create() -> TypeInfo {
        return {
            .element_size = sizeof(std::string::value_type),
            .type = &typeid(std::string),
            .shadowType = &typeid(char*),
            .isInternalDataTriviallyCopyable = true,
        };
    }

    // General case
    template<typename T>
    static consteval auto Create() -> TypeInfo {
        return {
            .element_size = sizeof(T),
            .type = &typeid(T),
            .isTriviallyCopyable = std::is_trivially_copyable<T>::value,
            .isFundamental = std::is_fundamental<T>::value
        };
    }

    std::size_t element_size = 0;
    std::optional<int> nbr_elements; // Some types can include number of elements, for example arrays
    std::type_info const *type = nullptr;
    std::type_info const *shadowType = nullptr; // Some types are better handled like if they were another type: std::string/char*
    bool isTriviallyCopyable = false;
    bool isInternalDataTriviallyCopyable = false; // The internal data of for example std::vector
    bool isFundamental = false;
    bool isPathSpace = false;
    bool isArray = false;
};

struct InReference {
    static InReference PathSpace() {
        return {};
    }

    InReference() = default;
    InReference(void const *data, int const size, std::type_info const *info) : data(data), size(size), info(info) {};

    // std::vector<scalar>
    template<typename T>
    requires std::same_as<T, std::vector<typename T::value_type>> && std::is_scalar<typename T::value_type>::value
    InReference(T const &t) : data(static_cast<void const*>(t.data())),
                              size(t.size()*sizeof(typename T::value_type)),
                              info(&typeid(int)),
                              isTriviallyCopyable(true),
                              isFundamental(true) {
                                int a = 0;
                                a++;
                              }
    // Fundamental
    template<typename T>
    requires std::is_fundamental<T>::value
    InReference(T const &t) : data(static_cast<void const*>(&t)),
                              size(sizeof(T)),
                              info(&typeid(T)),
                              isTriviallyCopyable(true),
                              isFundamental(true) {
                                int a = 0;
                                a++;
                              }
    // std::string
    template<typename T>
    requires std::same_as<T, std::string>
    InReference(T const &t) : data(static_cast<void const*>(t.data())),
                              size(t.size()),
                              info(&typeid(char*)),
                              isTriviallyCopyable(true),
                              isFundamental(true) {
                                int a = 0;
                                a++;
                              }
    // char array
    template <std::size_t N>
    InReference(char const (&t)[N]) : data(static_cast<void const*>(&t)),
                                      size(N-1),
                                      info(&typeid(char*)),
                                      isTriviallyCopyable(true),
                                      isFundamental(true) {
                                        int a = 0;
                                        a++;
                                      }
    // array of fundamental type
    template <typename T, std::size_t N>
    requires std::is_fundamental_v<T>
    InReference(T const (&t)[N]) : data(static_cast<void const*>(&t)),
                                   size(N*sizeof(T)),
                                   info(&typeid(T)),
                                   isTriviallyCopyable(true),
                                   isFundamental(true),
                                   isArray(true) {
                                        int a = 0;
                                        a++;
                                   }
    // Non-fundamental types that are JSON convertable and trivially copyable
    template<typename T>
    requires has_json_conversion<T> && (!std::is_fundamental_v<T>) && std::is_trivially_copyable<T>::value
    InReference(T const &t) : data(static_cast<void const*>(&t)),
                              size(sizeof(T)),
                              info(&typeid(T)),
                              isTriviallyCopyable(true),
                              isFundamental(false) {
        bool a = std::is_trivially_copyable<T>::value;
        if(!Converters::triviallyCopyableToJSON.contains(this->info)) {
            Converters::triviallyCopyableToJSON[this->info] = [](std::byte const *data) {
                nlohmann::json out;
                to_json(out, *reinterpret_cast<T const*>(data));
                return out;
            };
        }
    }
    // Non-fundamental types that are JSON convertable and not trivially copyable and has byte vector conversion
    template<typename T>
    requires has_json_conversion<T> && (!std::is_fundamental_v<T>) && (!std::is_trivially_copyable<T>::value) && has_byte_vector_conversion<T>
    InReference(T const &t) : data(static_cast<void const*>(&t)),
                              size(sizeof(T)),
                              info(&typeid(T)),
                              isTriviallyCopyable(false),
                              isFundamental(false) {
        bool a = std::is_trivially_copyable<T>::value;
        if(!Converters::toJSON.contains(this->info)) {
            Converters::toJSON[this->info] = [](std::byte const *data, int size) {
                nlohmann::json out;
                T obj;
                Converters::fromByteArray[&typeid(T)](data, &obj);
                to_json(out, obj);
                return out;
            };
        }
        if(!Converters::toByteArray.contains(this->info)) {
            Converters::toByteArray[this->info] = [](std::vector<std::byte> &vec, void const *obj) {
                to_bytevec(vec, *static_cast<T const*>(obj));
            };
        }
        if(!Converters::fromByteArray.contains(this->info)) {
            Converters::fromByteArray[this->info] = [](std::byte const *fromBytes, void *toObj) {
                try {
                    from_bytevec(fromBytes, *static_cast<T*>(toObj));
                } catch(std::exception const &e) {
                    return false;
                }
                return true;
            };
        }
    }
    // Non-fundamental types that are JSON convertable and not trivially copyable but does not have byte vector conversion
    template<typename T>
    requires has_json_conversion<T> && (!std::is_fundamental_v<T>) && (!std::is_trivially_copyable<T>::value) && (!has_byte_vector_conversion<T>)
    InReference(T const &t) : data(static_cast<void const*>(&t)),
                              size(sizeof(T)),
                              info(&typeid(T)),
                              isTriviallyCopyable(false),
                              isFundamental(false) {
        if(!Converters::toJSON.contains(this->info)) {
            Converters::toJSON[this->info] = [](std::byte const *data, int const size) {
                return nlohmann::json::from_bson(std::vector<std::byte>(data, data+size));
            };
        }
        if(!Converters::fromJSON.contains(this->info)) {
            Converters::fromJSON[this->info] = [](std::byte const *fromBytes, int const size, void *toObject) {
                try {
                    auto json = nlohmann::json::from_bson(std::vector<std::byte>(fromBytes, fromBytes+size));
                    *static_cast<T*>(toObject) = json.get<T>();
                } catch(std::exception const &e) {
                    return false;
                }
                return true;
            };
        }
        if(!Converters::toCompressedByteArray.contains(this->info)) {
            Converters::toCompressedByteArray[this->info] = [](std::vector<std::byte> &vec, void const *obj) {
                nlohmann::json out;
                to_json(out, *static_cast<T const*>(obj));
                std::vector<std::uint8_t> v_bson = nlohmann::json::to_bson(out);
                copy_byte_back_insert(v_bson.data(), v_bson.size(), vec);
            };
        }
    }
    // PathSpace
    template<IsPathSpace2 T>
    InReference(T const &t) : data(static_cast<void const*>(&t)),
                              size(sizeof(T)),
                              info(&typeid(T)),
                              isTriviallyCopyable(false),
                              isFundamental(false),
                              isPathSpace(true) {
                                int a = 0;
                                a++;
                              }
    // Other types
    template<typename T>
    InReference(T const &t) : data(static_cast<void const*>(&t)),
                              size(sizeof(T)),
                              info(&typeid(T)),
                              isTriviallyCopyable(std::is_trivially_copyable<T>::value),
                              isFundamental(false) {
                                int a = 0;
                                a++;
                              }

    void const *data = nullptr;
    unsigned int size = 0;
    std::type_info const *info = nullptr;
    bool isTriviallyCopyable = false;
    bool isFundamental = false;
    bool isPathSpace = false;
    bool isArray = false;
    TypeInfo info_;
};
} 