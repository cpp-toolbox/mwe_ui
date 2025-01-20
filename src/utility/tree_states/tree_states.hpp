#ifndef TREE_STATES_HPP
#define TREE_STATES_HPP

enum class TreeState {
    // TODO remove the dummy state and fill in with all the states you need
    MAIN_MENU,

    SETTINGS,
    PLAYER,
    INPUT,
    SOUND,
    GRAPHICS,
    ADVANCED,

    CREDITS,
    IN_GAME,
};

const char *to_string(TreeState state);

#endif // TREE_STATES_HPP
