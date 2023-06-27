#pragma once

inline auto Scroll::spacesToJSON() const -> nlohmann::json {
    nlohmann::json json;
    for(auto const &space : this->spaces)
        json += space->toJSON();
    return json;
}

inline auto Scroll::insert(Path const &range, InReference const &inref) -> bool {
    if(this->spaces.empty())
        return false;
    return this->spaces.front()->insert(range, inref);
}

inline auto Scroll::grab(Path const &range, void *obj, std::type_info const *info) -> bool {
    if(range.isAtSpace()) {
        if(this->spaces.empty())
            return false;
        //*static_cast<PathSpace2*>(obj) = *this->spaces.front();
        return true;
    } else {
       // if(this->scrolls.count(info))
         //   return this->scrolls.at(info).grab(range, obj, info);
    }
    return false;
}