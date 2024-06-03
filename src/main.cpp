#include <iostream>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <gfx/renderer.h>

int main(int, char**) {
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
  renderer.init(gfx::InitInfo{ SDL_GL_GetProcAddress });

  const char* vs_src = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

  const char* fs_src = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";
  
  gfx::Shader shader = renderer.new_shader(
    gfx::ShaderDesc{
      vs_src,
      fs_src,
    }
  );

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

    renderer.begin_default_pass(
      gfx::PassAction{
        gfx::ColorAction {
          .color = gfx::Color(0.5, 0.0, 0.0, 0.0)
        }
      }
    );
    renderer.apply_pipeline(pipe);
    renderer.apply_bindings(bind);
    //renderer.apply_uniforms(uniforms);
    renderer.draw(0, 3, 1);

    SDL_GL_SwapWindow(window);
  }

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}