#pragma once

typedef struct SDL_Window SDL_Window;

namespace core {
  struct InitInfo {
    SDL_Window* window = nullptr;
  };

  class Engine {
  public:
    void init(const InitInfo& info);
    void shutdown();
    void tick();
  };
}
