#ifndef ILMENDUR_SAVE_SYSTEM_HPP
#define ILMENDUR_SAVE_SYSTEM_HPP

namespace Core {
    struct GameState;
}

namespace SaveSystem {

    void load(unsigned int slot, Core::GameState& gs);
    void save(unsigned int slot, const Core::GameState& gs);
};

#endif /* ILMENDUR_SAVE_SYSTEM_HPP */
