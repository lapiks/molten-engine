#include <iostream>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

int main(int, char**) {
  std::cout << "Hello, world!" << std::endl;

  SDL_SetMainReady();
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "Could not initialize SDL! Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  int windowFlags = 0;
  SDL_Window* window = SDL_CreateWindow("Molten Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, windowFlags);

  bool shouldClose = false;
  while (!shouldClose) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        shouldClose = true;
        break;

      default:
        break;
      }
    }
  }

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}