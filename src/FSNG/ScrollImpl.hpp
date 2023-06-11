#pragma once

auto Scroll::spacesToJSON() const -> nlohmann::json {
    nlohmann::json json;
    for(auto const &space : this->spaces)
        json += space->toJSON();
    return json;
}

auto Scroll::insert(Path const &range, InReference const &inref) -> bool {
    if(this->spaces.empty())
        return false;
    return this->spaces.front()->insert(range, inref);
}