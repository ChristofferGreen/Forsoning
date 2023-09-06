#pragma once

inline auto Codex2::insert(Path const &range, InReference const &inref) -> bool {
    if(!this->scrolls.contains(&typeid(PathSpace2)))
        this->scrolls.emplace(&typeid(PathSpace2), Scroll(inref));
    return this->scrolls.at(&typeid(PathSpace2)).insert(range, inref);
}

inline auto Codex2::grab(Path const &range, void *obj, std::type_info const *type) -> bool {
    if(range.isAtSpace()) {
        if(this->scrolls.count(&typeid(PathSpace2)))
            return this->scrolls.at(&typeid(PathSpace2)).grab(range, obj, type);
    } else {
        if(type==&typeid(std::string) && this->scrolls.count(&typeid(char*)))
            return this->scrolls.at(&typeid(char*)).grab(range, obj, type);
        if(this->scrolls.count(type))
            return this->scrolls.at(type).grab(range, obj, type);
    }
    return false;
}