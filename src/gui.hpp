#ifndef ILMENDUR_GUI_HPP
#define ILMENDUR_GUI_HPP
#include <string>
#include <initializer_list>
#include <functional>
#include <SDL2/SDL.h>

/**
 * Interface to the Ilmendur-specific GUI elements. This module should
 * be used whenever standardised GUI elements such as dialog boxes are
 * required. Its elements are rendered and its events are read
 * directly within the Ilmendur main loop function, that is, it is not
 * necessary to call update() or handleEvent() within any scene. Use
 * the other methods of this namespace to create such GUI elements.
 *
 * The methods in this namespace depart from ImGui's "immediate" paradigm.
 * Only call them *once* to construct the GUI element in question;
 * the update() function, which is called in the main loop directly,
 * will take care of advising ImGui to draw the element as long as
 * necessary.
 */
namespace GUISystem {
    void loadFonts();
    void update();
    bool handleEvent(const SDL_Event& event);
    void messageDialog(unsigned int playerno, std::initializer_list<std::string> texts, std::function<void()> callback);
}

#endif /* ILMENDUR_GUI_HPP */
