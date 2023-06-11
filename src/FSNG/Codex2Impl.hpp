#pragma once

auto Codex2::insert(Path const &range, InReference const &inref) -> bool {
    return this->scrolls[&typeid(PathSpace2)].insert(range, inref);
}