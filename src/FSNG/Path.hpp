#pragma once
#include <filesystem>
#include <optional>

namespace FSNG {
struct Path {
    Path() = default;
    Path(std::filesystem::path const &path) : path(path) {
        this->current = this->path.begin();
        this->end   = this->path.end();
        if(this->current!=this->end)
            this->end--; // now at data component
        if(*this->end=="/") // we only had '/'
            this->valid = false;
        if(*this->current=="/") // Sometimes we get a / in the start, remove it
            this->current++;
    }
    Path(const char* const &str) : Path(std::filesystem::path(str)) {}
    Path(std::string const &str) : Path(std::filesystem::path(str)) {}
    Path(std::filesystem::path::const_iterator const &current, std::filesystem::path::const_iterator const &end) : current(current), end(end) {}

    bool operator==(Path const &rhs) const {
        return this->path==rhs.path;
    }

    bool operator!=(Path const &rhs) const {
        return this->path!=rhs.path;
    }

    bool operator<(Path const &rhs) const {
        return this->path<rhs.path;
    }
    
    bool operator>(Path const &rhs) const {
        return this->path>rhs.path;
    }

    auto original() const -> Path {
        return Path(this->path);
    }

    auto next() const {
        auto val = *this;
        val.current++;
        val.level++;
        return val;
    }

    auto spaceName() const -> std::optional<std::string> {
        if(*this->current=="")
            return std::nullopt;
        return (*this->current).string();
    }

    auto string() const -> std::string {
        return this->path.string();
    }

    auto dataName() const -> std::string {
        return (*this->end).string();
    }

    auto isAtData() const -> bool {
        return current==end;
    }

    auto isAtRoot() const -> bool {
        return this->level==0;
    }

    auto empty() const -> bool {
        return this->path.empty();
    }

    auto isValid() -> bool {
        return this->valid;
    }
private:
    std::filesystem::path path;
    bool valid = true;
    std::filesystem::path::const_iterator current, end;
    int level = 0;
};
}