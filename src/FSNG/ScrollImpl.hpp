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

inline auto Scroll::grab(Path const &range, void *obj, std::type_info const *type, std::size_t const size) -> bool {
    if(range.isAtSpace()) {
        if(this->spaces.empty())
            return false;
        //*static_cast<PathSpace2*>(obj) = *this->spaces.front();
        return true;
    }
    if(this->fromByteVec) {
        this->fromByteVec(obj, this->data.data(), size);
        if(this->itemSizes.size()) {
            this->data.erase(this->data.begin(), this->data.begin() + *this->itemSizes.begin());
            this->itemSizes.erase(this->itemSizes.begin(), this->itemSizes.begin() + 1);
        }
    }
    /*if(this->grab_builtin<char>(obj, *type)) return true;
    else if(this->grab_builtin<signed char>(obj, *type)) return true;
    else if(this->grab_builtin<unsigned char>(obj, *type)) return true;
    else if(this->grab_builtin<short>(obj, *type)) return true;
    else if(this->grab_builtin<short int>(obj, *type)) return true;
    else if(this->grab_builtin<signed short>(obj, *type)) return true;
    else if(this->grab_builtin<signed short int>(obj, *type)) return true;
    else if(this->grab_builtin<int>(obj, *type)) return true;
    else if(this->grab_builtin<signed>(obj, *type)) return true;
    else if(this->grab_builtin<signed int>(obj, *type)) return true;
    else if(this->grab_builtin<unsigned>(obj, *type)) return true;
    else if(this->grab_builtin<unsigned int>(obj, *type)) return true;
    else if(this->grab_builtin<long>(obj, *type)) return true;
    else if(this->grab_builtin<long int>(obj, *type)) return true;
    else if(this->grab_builtin<signed long>(obj, *type)) return true;
    else if(this->grab_builtin<signed long int>(obj, *type)) return true;
    else if(this->grab_builtin<unsigned long>(obj, *type)) return true;
    else if(this->grab_builtin<unsigned long int>(obj, *type)) return true;
    else if(this->grab_builtin<long long>(obj, *type)) return true;
    else if(this->grab_builtin<long long int>(obj, *type)) return true;
    else if(this->grab_builtin<signed long long int>(obj, *type)) return true;
    else if(this->grab_builtin<unsigned long int>(obj, *type)) return true;
    else if(this->grab_builtin<double>(obj, *type)) return true;
    else if(this->grab_builtin<long double>(obj, *type)) return true;
    else if(this->grab_builtin<bool>(obj, *type)) return true;
    else if(this->grab_builtin<wchar_t>(obj, *type)) return true;
    else if(this->grab_builtin<char*>(obj, *type)) return true;*/
    return false;
}