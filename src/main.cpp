#include <iostream>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <gfx/renderer.h>

int main(int, char**) {
  std::cout << "Hello, world!" << std::endl;

  SDL_SetMainReady();
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "Failed to initialize SDL. Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  int windowFlags = SDL_WINDOW_OPENGL;
  SDL_Window* window = SDL_CreateWindow("Molten Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, windowFlags);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  if (!gl_context) {
    std::cerr << "Failed to initialize GL context. Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  // vsync
  SDL_GL_SetSwapInterval(1);
  gfx::Renderer renderer;
  renderer.init(SDL_GL_GetProcAddress);

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

    renderer.draw();

    SDL_GL_SwapWindow(window);
  }

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}