#pragma once
#include <filesystem>
#include <optional>

namespace FSNG {
struct Security {
    enum struct Type {
        Grab = 0,
        Insert
    };

    struct Policy {
        using FunT = std::function<bool(Type const &sec, Path const &path)>;
        inline static const FunT AlwaysAllow  = [](Type const &sec, Path const &path) noexcept { return true; };
        inline static const FunT AlwaysReject = [](Type const &sec, Path const &path) noexcept { return false; };
    };

    struct Key {
        Key() = default;
        Key(Policy::FunT const &policy) : policy(policy) {}
        
        auto allow(Type const &type, Path const &path) const noexcept { return this->policy(type, path); }

        private:
            Policy::FunT policy = Policy::AlwaysAllow;
    };
};
}