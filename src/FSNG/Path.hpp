#pragma once
#include <filesystem>
#include <optional>

namespace FSNG {
struct Path {
    Path() = default;
    Path(std::filesystem::path const &path) : path(path) {}

    struct Range {
        Range() = default;
        Range(std::filesystem::path::const_iterator const &current, std::filesystem::path::const_iterator const &end) : current(current), end(end) {}

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

        auto dataName() const -> std::string {
            return *this->end;
        }

        auto isAtData() const -> bool {
            return current==end;
        }
    private:
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