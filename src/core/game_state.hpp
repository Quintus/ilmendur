/**
 * Global game state header. Declares data structures required all
 * over the game. Since this file is included in many places, keep its
 * dependencies as low as possible.
 */

#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

namespace Core {

    /**
     * Constants for all memory structures that store data
     * about the two protagonists. This cannot be an "enum
     * class", because they are used for array numbering,
     * which would require casting their values all the time.
     * Effectively, these are simply array indices.
     */
    enum character {
        BENJAMIN = 0,
        FREYA = 1
    };

    /**
     * Constants for all memory structures that relate to data
     * about the human players. This includes most notably the
     * gamepad configuration. This cannot be an
     * "enum class", because these values are used for array
     * numbering, which would require casting their values
     * all the time.
     */
    enum player {
        PLAYER1 = 0,
        PLAYER2 = 1
    };

    /**
     * Information about a single joystick axis, being
     * the axes index for GLFW and whether the axis is
     * inverted.
     */
    struct axisconf {
        int axisno = 0;
        bool inverted = false;
    };

    /**
     * Global game state object. This nested structure is what is
     * stored inside save game files. It is globally accessible
     * via its GameState::instance member.
     *
     * When adding to this structure, please always specify
     * C++11 default values.
     */
    struct GameState {
        unsigned int version = 1;

        struct {
            bool cleared_dungeon_01 = false;
        } achievments;
        struct {
            int bombs = 0;
        } equipment[2]; // one equipment struct per character; use "character" enum indices for access.
        struct {
            int joy_index = 0;
            axisconf joy_vertical;
            axisconf joy_horizontal;
            axisconf joy_cam_vertical;
            axisconf joy_cam_horizontal;
            float joy_dead_zone = 0.0f; ///< Dead zone, as a value between 0 (no dead zone) and 1 (everything). Dead zone is the zone in which axis input is ignored.
        } config[2]; // One configuration struct per player; use "player" enum indices for access.

        static GameState instance;
    };

}

#endif /* GAME_STATE_HPP */
