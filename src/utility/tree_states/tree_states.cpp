#include "tree_states.hpp"

const char *to_string(TreeState state) {
    switch (state) {
    case TreeState::MAIN_MENU:
        return "MAIN_MENU";

    case TreeState::SETTINGS:
        return "SETTINGS";
    case TreeState::PLAYER:
        return "PLAYER";
    case TreeState::INPUT:
        return "INPUT";
    case TreeState::SOUND:
        return "SOUND";
    case TreeState::GRAPHICS:
        return "GRAPHICS";
    case TreeState::ADVANCED:
        return "ADVANCED";

    case TreeState::CREDITS:
        return "CREDITS";
    case TreeState::IN_GAME:
        return "IN_GAME";
    default:
        return "UNKNOWN";
    }
}
