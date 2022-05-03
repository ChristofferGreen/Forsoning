#include <doctest.h>

#include "PathSpace.hpp"
#include <iostream>

using namespace FSNG;

struct POD {
    bool operator==(POD const&) const = default;
    int a = 13;
    float b = 44.0;
};

inline void to_json(nlohmann::json& j, const POD& p) {
    j = nlohmann::json{{"a", p.a}, {"b", p.b}};
}

struct NonTrivial {
    int a = 13;
    std::vector<int> b;
};

inline void to_json(nlohmann::json& j, const NonTrivial& p) {
    j = nlohmann::json{{"a", p.a}, {"b", p.b}};
}

inline void to_bytevec(std::vector<std::byte> &vec, NonTrivial const &obj) {
    copy_byte_back_insert(&obj.a, sizeof(int), vec);
    int const elements = obj.b.size();
    copy_byte_back_insert(&elements, sizeof(int), vec);
    copy_byte_back_insert(obj.b.data(), sizeof(int)*obj.b.size(), vec);
}

inline void from_bytevec(std::byte const *vec, NonTrivial &ret) {
    ret.a = *reinterpret_cast<int const*>(vec);
    vec += sizeof(int);
    int const elements = *reinterpret_cast<int const*>(vec);
    vec += sizeof(int);
    for(auto i = 0; i < elements; ++i) {
        auto const val = *reinterpret_cast<int const *>(vec);
        ret.b.push_back(val);
        vec += sizeof(int);
    }
}

struct NonTrivialJS {
    int a = 13;
    std::vector<int> b;
};

inline void to_json(nlohmann::json& j, const NonTrivialJS& p) {
    j = nlohmann::json{{"a", p.a}, {"b", p.b}};
}
