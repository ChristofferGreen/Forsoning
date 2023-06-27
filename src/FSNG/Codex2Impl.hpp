#pragma once

inline auto Codex2::insert(Path const &range, InReference const &inref) -> bool {
    return this->scrolls[&typeid(PathSpace2)].insert(range, inref);
}

inline auto Codex2::grab(Path const &range, void *obj, std::type_info const *info) -> bool {
    if(range.isAtSpace()) {
        if(this->scrolls.count(&typeid(PathSpace2)))
            return this->scrolls.at(&typeid(PathSpace2)).grab(range, obj, info);
    } else {
        if(this->scrolls.count(info))
            return this->scrolls.at(info).grab(range, obj, info);
    }
    return false;
}