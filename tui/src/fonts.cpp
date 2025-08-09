#include <unordered_map>

#include "engine/types.hpp"

#include "tui/fonts.hpp"
#include "tui/types.hpp"

namespace tui {

std::unordered_map<char, PieceFontMap> kPieceFontsMap = {
    {'r', {kRook, engine::BLACK}},   {'n', {kKnight, engine::BLACK}},
    {'b', {kBishop, engine::BLACK}}, {'k', {kKing, engine::BLACK}},
    {'q', {kQueen, engine::BLACK}},  {'p', {kPawn, engine::BLACK}},
    {'R', {kRook, engine::WHITE}},   {'N', {kKnight, engine::WHITE}},
    {'B', {kBishop, engine::WHITE}}, {'K', {kKing, engine::WHITE}},
    {'Q', {kQueen, engine::WHITE}},  {'P', {kPawn, engine::WHITE}},
};

}
