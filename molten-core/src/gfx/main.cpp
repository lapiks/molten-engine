#include <iostream>
#include <chrono>
#include <thread>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <glm/glm.hpp>

#include "renderer.h"

#define USE_OPENGL
//#define USE_VULKAN

struct BasicShader {
  static inline const char* VERTEX = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

  static inline const char* FRAGMENT = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

  struct Uniforms {
    glm::mat4 mvp;
  };

  static gfx::ShaderDesc desc() {
    return gfx::ShaderDesc{
      .vertex_src = VERTEX,
      .fragment_src = FRAGMENT,
      .uniforms_layout = gfx::UniformBlockLayout {
        .uniforms = {
          gfx::UniformDesc {
            .name = "MVP",
            .type = gfx::UniformType::MAT4,
          }
        },
      }
    };
  }
};

int main(int, char**) {
  SDL_SetMainReady();
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "Failed to initialize SDL. Error: " << SDL_GetError() << std::endl;
    return 1;
  }

#ifdef USE_OPENGL
  int windowFlags = SDL_WINDOW_OPENGL;
#else 
  int windowFlags = SDL_WINDOW_VULKAN;
#endif
  SDL_Window* window = SDL_CreateWindow(
    "Molten Engine", 
    SDL_WINDOWPOS_UNDEFINED, 
    SDL_WINDOWPOS_UNDEFINED, 
    1024, 
    680, 
    windowFlags
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
    -0.5f, -0.5f, 0.0f, // left  
     0.5f, -0.5f, 0.0f, // right 
     0.0f,  0.5f, 0.0f  // top   
  };

  gfx::Buffer vbuffer = renderer.new_buffer(
    gfx::BufferDesc{
      gfx::MAKE_MEMORY(vertices),
      gfx::BufferType::VERTEX_BUFFER,
    }
    );

  uint16_t indices[] = {
    1, 2, 3
  };

  gfx::Buffer ibuffer = renderer.new_buffer(
    gfx::BufferDesc{
      gfx::MAKE_MEMORY(indices),
      gfx::BufferType::INDEX_BUFFER,
    }
    );

  gfx::VertexLayout layout;
  layout.attributes[0].format = gfx::AttributeFormat::FLOAT3;

  gfx::Pipeline pipe = renderer.new_pipeline(
    gfx::PipelineDesc{
      shader,
      layout,
      gfx::IndexType::UINT16,
    }
    );

  gfx::Bindings bind;
  bind.vertex_buffers[0] = vbuffer;
  bind.index_buffer = ibuffer;

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

    renderer.begin_default_pass(
      gfx::PassAction{
        gfx::ColorAction {
          .color = gfx::Color(0.5, 0.0, 0.0, 0.0)
        }
      }
    );
    renderer.set_pipeline(pipe);
    renderer.set_bindings(bind);
    renderer.set_viewport({ 0, 0, 500, 500 });
    renderer.set_scissor({ 0, 0, 500, 500 });
    BasicShader::Uniforms uniforms{
      .mvp = glm::mat4(1.0),
    };
    renderer.set_uniforms(gfx::ShaderStage::VERTEX, gfx::MAKE_MEMORY(uniforms));
    renderer.draw(0, 3, 1);

    SDL_GL_SwapWindow(window);
  }

  renderer.shutdown();
#ifdef USE_OPENGL
  SDL_GL_DeleteContext(gl_context);
#endif
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}