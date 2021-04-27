#ifndef RPG_SAVE_SYSTEM_HPP
#define RPG_SAVE_SYSTEM_HPP

namespace Core {
    struct GameState;
}

namespace SaveSystem {

    void load(unsigned int slot, Core::GameState& gs);
    void save(unsigned int slot, const Core::GameState& gs);
};

#endif /* RPG_SAVE_SYSTEM_HPP */
