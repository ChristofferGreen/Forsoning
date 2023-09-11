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

template<typename T>
inline auto grab_built_in(std::vector<std::byte> &data, void *obj, std::type_info const &type) -> bool {
    /* Todo: Have a start index that gets moved on every grab instead of erasing from vector.
                When the start index icrosses the halfway point->reallocate vector.
    */
    if(data.size()==0)
        return false;
    if(type==typeid(std::string) && typeid(T)==typeid(char*)) {
        /**reinterpret_cast<std::string*>(obj) = std::string(reinterpret_cast<char const*>(data.data()), *this->itemSizes.begin());
        data.erase(data.begin(), data.begin() + sizeof(T));
        this->itemSizes.erase(data.begin(), data.begin() + 1);*/
        return true;
    }
    else if(type==typeid(T)) {
        *reinterpret_cast<T*>(obj) = *reinterpret_cast<T const*>(data.data());
        data.erase(data.begin(), data.begin() + sizeof(T));
        //data.itemSizes.erase(data.begin(), data.begin() + 1);
        return true;
    }
    return false;
}

inline auto Scroll::grab(Path const &range, void *obj, std::type_info const *type) -> bool {
    if(range.isAtSpace()) {
        if(this->spaces.empty())
            return false;
        //*static_cast<PathSpace2*>(obj) = *this->spaces.front();
        return true;
    }
    if(grab_built_in<char>(this->data, obj, *type)) return true;
    else if(grab_built_in<signed char>(this->data, obj, *type)) return true;
    else if(grab_built_in<unsigned char>(this->data, obj, *type)) return true;
    else if(grab_built_in<short>(this->data, obj, *type)) return true;
    else if(grab_built_in<short int>(this->data, obj, *type)) return true;
    else if(grab_built_in<signed short>(this->data, obj, *type)) return true;
    else if(grab_built_in<signed short int>(this->data, obj, *type)) return true;
    else if(grab_built_in<int>(this->data, obj, *type)) return true;
    else if(grab_built_in<signed>(this->data, obj, *type)) return true;
    else if(grab_built_in<signed int>(this->data, obj, *type)) return true;
    else if(grab_built_in<unsigned>(this->data, obj, *type)) return true;
    else if(grab_built_in<unsigned int>(this->data, obj, *type)) return true;
    else if(grab_built_in<long>(this->data, obj, *type)) return true;
    else if(grab_built_in<long int>(this->data, obj, *type)) return true;
    else if(grab_built_in<signed long>(this->data, obj, *type)) return true;
    else if(grab_built_in<signed long int>(this->data, obj, *type)) return true;
    else if(grab_built_in<unsigned long>(this->data, obj, *type)) return true;
    else if(grab_built_in<unsigned long int>(this->data, obj, *type)) return true;
    else if(grab_built_in<long long>(this->data, obj, *type)) return true;
    else if(grab_built_in<long long int>(this->data, obj, *type)) return true;
    else if(grab_built_in<signed long long int>(this->data, obj, *type)) return true;
    else if(grab_built_in<unsigned long int>(this->data, obj, *type)) return true;
    else if(grab_built_in<double>(this->data, obj, *type)) return true;
    else if(grab_built_in<long double>(this->data, obj, *type)) return true;
    else if(grab_built_in<bool>(this->data, obj, *type)) return true;
    else if(grab_built_in<wchar_t>(this->data, obj, *type)) return true;
    else if(grab_built_in<char*>(this->data, obj, *type)) return true;
    return false;
}