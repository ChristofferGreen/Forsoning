#pragma once
#include <filesystem>
#include <optional>

namespace FSNG {
struct Path {
    Path() = default;
    Path(std::filesystem::path const &path) : path(path) {}

    struct Range {
        auto next() const {
            auto val = *this;
            val.current++;
            return val;
        }

        auto spaceName() const -> std::optional<std::string> {
            if(*this->current=="")
                return std::nullopt;
            return *this->current;
        }

        std::filesystem::path::const_iterator current, end;
    };
    
    auto range() const -> std::optional<Range> {
        auto start = this->path.begin();
        auto end   = this->path.end();
        end--; // now at data component
        if(*end=="/") // we only had '/'
            return std::nullopt;
        if(*start=="/") // Sometimes we get a / in the start, remove it
            start++;
        if(start==end) // We only have data component
            return std::nullopt;
        return Range{start, end};
    }

    auto dataName() const -> std::optional<std::string> {
        auto fname = this->path.filename();
        if(fname=="")
            return std::nullopt;
        return fname;
    }

    std::filesystem::path path;
};
}