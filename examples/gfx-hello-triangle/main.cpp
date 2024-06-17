#include <iostream>
#include <chrono>
#include <thread>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gfx/renderer.h"
#include "image.h"
#include "engine.h"

#define USE_OPENGL
//#define USE_VULKAN

struct BasicShader {
  static inline const char* VERTEX = "#version 330 core\n"
    "layout (location = 0) in vec3 a_pos;\n"
    "layout (location = 1) in vec4 a_color;\n"
    "out vec4 io_color;\n"
    "void main()\n"
    "{\n"
    "   io_color = a_color;\n"
    "   gl_Position = vec4(a_pos, 1.0);\n"
    "}\0";

  static inline const char* FRAGMENT = "#version 330 core\n"
    "in vec4 io_color;\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = io_color;\n"
    "}\n\0";

  static gfx::ShaderDesc desc() {
    return gfx::ShaderDesc{
      .vertex_src = VERTEX,
      .fragment_src = FRAGMENT,
    };
  }
};

int main(int, char**) {
  SDL_SetMainReady();
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "Failed to initialize SDL. Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  uint32_t window_width = 1024;
  uint32_t window_height = 680;

#ifdef USE_OPENGL
  int windowFlags = SDL_WINDOW_OPENGL;
#else 
  int windowFlags = SDL_WINDOW_VULKAN;
#endif
  SDL_Window* window = SDL_CreateWindow(
    "Molten Engine",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    window_width,
    window_height,
    windowFlags | SDL_WINDOW_RESIZABLE
  );
  if (!window) {
    std::cerr << "Failed to initialize SDL window. Error: " << SDL_GetError() << std::endl;
    return 1;
  }

#ifdef USE_OPENGL
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
#endif

  gfx::Renderer renderer;
  renderer.init(gfx::InitInfo{ window });

  gfx::Shader shader = renderer.new_shader(BasicShader::desc());

  float vertices[] = {
    /* pos               color */              
    0.0f, 0.5f, 0.0f,    1.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.0f,    0.0f, 1.0f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.0f,    0.0f, 0.0f, 1.0f, 1.0f,
  };

  gfx::Buffer vbuffer = renderer.new_buffer(
    gfx::BufferDesc{
      gfx::MAKE_MEMORY(vertices),
      gfx::BufferType::VERTEX_BUFFER,
    }
    );

  gfx::VertexLayout layout;
  layout.attributes[0].format = gfx::AttributeFormat::FLOAT3;
  layout.attributes[1].format = gfx::AttributeFormat::FLOAT4;

  gfx::Pipeline pipe = renderer.new_pipeline(
    gfx::PipelineDesc{
      .shader = shader,
      .layout = layout,
      .index_type = gfx::IndexType::NONE,
      .cull = gfx::CullMode::BACK,
    }
    );

  gfx::Bindings bind{
    .vertex_buffer = vbuffer,
  };

  bool should_close = false;
  bool stop_rendering = false;
  while (!should_close) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        should_close = true;
        break;

      case SDL_WINDOWEVENT: {
        if (event.window.event == SDL_WINDOWEVENT_MINIMIZED) {
          stop_rendering = true;
        }
        if (event.window.event == SDL_WINDOWEVENT_RESTORED) {
          stop_rendering = false;
        }
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          window_width = event.window.data1;
          window_height = event.window.data2;
          renderer.set_viewport({ 0, 0, window_width, window_height });
        }
      }
                          break;

      default:
        break;
      }
    }

    if (stop_rendering) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }

    renderer.begin_default_render_pass(
      gfx::PassAction{
        gfx::ColorAction {
          .color = gfx::Color(0.1f, 0.1f, 0.1f, 1.0f),
        }
      }
    );
    renderer.set_pipeline(pipe);
    renderer.set_bindings(bind);
    renderer.draw(0, 3, 1);
    renderer.end_render_pass();

    renderer.submit();

#ifdef USE_OPENGL
    SDL_GL_SwapWindow(window);
#endif
  }

  renderer.shutdown();
#ifdef USE_OPENGL
  SDL_GL_DeleteContext(gl_context);
#endif
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}