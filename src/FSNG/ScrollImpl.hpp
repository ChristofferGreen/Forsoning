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

inline auto Scroll::grab(Path const &range, void *obj, std::type_info const *type) -> bool {
    if(range.isAtSpace()) {
        if(this->spaces.empty())
            return false;
        //*static_cast<PathSpace2*>(obj) = *this->spaces.front();
        return true;
    } else {
        if(*type==typeid(int)) {
            int &i = *reinterpret_cast<int*>(obj);
            i = *reinterpret_cast<int const*>(this->data.data());
            this->data.erase(this->data.begin(), this->data.begin() + 4);
            return true;
        }
    }
    return false;
}