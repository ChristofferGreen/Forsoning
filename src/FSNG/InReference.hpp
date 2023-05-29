#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>

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

struct InReference {
    InReference() = default;
    InReference(void const *data, int const size, std::type_info const *info) : data(data), size(size), info(info) {};

    // std::vector
    template<typename T>
    requires std::same_as<T, std::vector<typename T::value_type>> && std::is_fundamental<typename T::value_type>::value
    InReference(T const &t) : data(static_cast<void const*>(t.data())),
                              size(t.size()*sizeof(typename T::value_type)),
                              info(&typeid(int)),
                              isStandardLayout(std::is_standard_layout<T>::value),
                              isFundamental(true) {}
    // std::string
    template<typename T>
    requires std::same_as<T, std::string>
    InReference(T const &t) : data(static_cast<void const*>(t.data())),
                              size(t.size()),
                              info(&typeid(char*)),
                              isStandardLayout(std::is_standard_layout<T>::value),
                              isFundamental(true) {}
    // char array
    template <std::size_t N>
    InReference(char const (&t)[N]) : data(static_cast<void const*>(&t)),
                                      size(N-1),
                                      info(&typeid(char*)),
                                      isStandardLayout(true),
                                      isFundamental(true) {}
    // array of fundamental type
    template <typename T, std::size_t N>
    requires std::is_fundamental_v<T>
    InReference(T const (&t)[N]) : data(static_cast<void const*>(&t)),
                                   size(N*sizeof(T)),
                                   info(&typeid(T)),
                                   isStandardLayout(std::is_standard_layout<T>::value),
                                   isFundamental(true) {}
    // Non-fundamental types that are JSON convertable and standard layout
    template<typename T>
    requires has_json_conversion<T> && (!std::is_fundamental_v<T>) && std::is_standard_layout<T>::value
    InReference(T const &t) : data(static_cast<void const*>(&t)),
                              size(sizeof(T)),
                              info(&typeid(T)),
                              isStandardLayout(std::is_standard_layout<T>::value),
                              isFundamental(false) {
        if(!Converters::toJSONConverters.contains(this->info)) {
            Converters::toJSONConverters[this->info] = [](std::byte const *data, int const size) {
                nlohmann::json out;
                to_json(out, *reinterpret_cast<T const*>(data));
                return out;
            };
        }
        if(!Converters::toByteArrayConverters.contains(this->info)) {
            Converters::toByteArrayConverters[this->info] = [](std::vector<std::byte> &vec, void const *obj) {
                nlohmann::json out;
                to_json(out, *static_cast<T const*>(obj));
                std::vector<std::uint8_t> v_bson = nlohmann::json::to_bson(out);
                copy_byte_back_insert(v_bson.data(), v_bson.size(), vec);
            };
        }
    }
    // Other Non-fundamental types
    template<typename T>
    InReference(T const &t) : data(static_cast<void const*>(&t)),
                              size(sizeof(T)),
                              info(&typeid(T)),
                              isStandardLayout(std::is_standard_layout<T>::value),
                              isFundamental(false) {}

    void const *data = nullptr;
    unsigned int size = 0;
    std::type_info const *info = nullptr;
    bool isStandardLayout = false;
    bool isFundamental = false;
};
} 