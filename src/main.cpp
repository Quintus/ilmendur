#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init() failed" << std::endl;
        return 1;
    }
    if (IMG_Init(IMG_INIT_PNG) < 0) {
        std::cerr << "IMG_Init() failed" << std::endl;
    }

    SDL_Window* p_window = SDL_CreateWindow("test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    if (!p_window) {
        std::cerr << "SDL_CreateWindow() failed" << std::endl;
        return 1;
    }

    SDL_Surface* p_sf = SDL_GetWindowSurface(p_window);

    bool run = true;
    while (run) {
        SDL_Event ev;
        SDL_PollEvent(&ev);
        if (ev.type == SDL_QUIT) {
            run = false;
        }

        SDL_FillRect(p_sf, NULL, SDL_MapRGB( p_sf->format, 0x00, 0xFF, 0x00));
        SDL_UpdateWindowSurface(p_window);
    }

    std::cout << "Hello, world" << std::endl;

    SDL_DestroyWindow(p_window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
