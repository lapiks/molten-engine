#include <iostream>
#include <chrono>
#include <thread>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer.h"
#include "image.h"
#include "engine.h"

#define USE_OPENGL
//#define USE_VULKAN

struct BasicShader {
  static inline const char* VERTEX = "#version 330 core\n"
    "layout (location = 0) in vec3 a_pos;\n"
    "layout (location = 1) in vec4 a_color;\n"
    "layout (location = 2) in vec2 a_uv;\n"
    "uniform mat4 u_mvp;\n"
    "out vec4 io_color;\n"
    "out vec2 io_uv;\n"
    "void main()\n"
    "{\n"
    "   io_color = a_color;\n"
    "   io_uv = a_uv;\n"
    "   gl_Position = u_mvp * vec4(a_pos, 1.0);\n"
    "}\0";

  static inline const char* FRAGMENT = "#version 330 core\n"
    "in vec4 io_color;\n"
    "in vec2 io_uv;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D u_tex1;\n"
    "uniform sampler2D u_tex2;\n"
    "void main()\n"
    "{\n"
    "   FragColor = mix(texture(u_tex2, io_uv), texture(u_tex1, io_uv), 0.5);\n"
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
            .name = "u_mvp",
            .type = gfx::UniformType::MAT4,
          }
        },
      },
      .texture_names = { "u_tex1", "u_tex2" },
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
    /* pos               color                   uvs */
    -1.0, -1.0, -1.0,    1.0, 0.5, 0.5, 1.0,     0.0, 0.0,
     1.0, -1.0, -1.0,    1.0, 0.5, 0.5, 1.0,     1.0, 0.0,
     1.0,  1.0, -1.0,    1.0, 0.5, 0.5, 1.0,     1.0, 1.0,
    -1.0,  1.0, -1.0,    1.0, 0.5, 0.5, 1.0,     0.0, 1.0,

    -1.0, -1.0,  1.0,    0.5, 1.0, 0.5, 1.0,     0.0, 0.0,
     1.0, -1.0,  1.0,    0.5, 1.0, 0.5, 1.0,     1.0, 0.0,
     1.0,  1.0,  1.0,    0.5, 1.0, 0.5, 1.0,     1.0, 1.0,
    -1.0,  1.0,  1.0,    0.5, 1.0, 0.5, 1.0,     0.0, 1.0,

    -1.0, -1.0, -1.0,    0.5, 0.5, 1.0, 1.0,     0.0, 0.0,
    -1.0,  1.0, -1.0,    0.5, 0.5, 1.0, 1.0,     1.0, 0.0,
    -1.0,  1.0,  1.0,    0.5, 0.5, 1.0, 1.0,     1.0, 1.0,
    -1.0, -1.0,  1.0,    0.5, 0.5, 1.0, 1.0,     0.0, 1.0,

     1.0, -1.0, -1.0,    1.0, 0.5, 0.0, 1.0,     0.0, 0.0,
     1.0,  1.0, -1.0,    1.0, 0.5, 0.0, 1.0,     1.0, 0.0,
     1.0,  1.0,  1.0,    1.0, 0.5, 0.0, 1.0,     1.0, 1.0,
     1.0, -1.0,  1.0,    1.0, 0.5, 0.0, 1.0,     0.0, 1.0,

    -1.0, -1.0, -1.0,    0.0, 0.5, 1.0, 1.0,     0.0, 0.0,
    -1.0, -1.0,  1.0,    0.0, 0.5, 1.0, 1.0,     1.0, 0.0,
     1.0, -1.0,  1.0,    0.0, 0.5, 1.0, 1.0,     1.0, 1.0,
     1.0, -1.0, -1.0,    0.0, 0.5, 1.0, 1.0,     0.0, 1.0,

    -1.0,  1.0, -1.0,    1.0, 0.0, 0.5, 1.0,     0.0, 0.0,
    -1.0,  1.0,  1.0,    1.0, 0.0, 0.5, 1.0,     1.0, 0.0,
     1.0,  1.0,  1.0,    1.0, 0.0, 0.5, 1.0,     1.0, 1.0,
     1.0,  1.0, -1.0,    1.0, 0.0, 0.5, 1.0,     0.0, 1.0
  };

  gfx::Buffer vbuffer = renderer.new_buffer(
    gfx::BufferDesc{
      gfx::MAKE_MEMORY(vertices),
      gfx::BufferType::VERTEX_BUFFER,
    }
    );

  uint16_t indices[] = {
    0, 1, 2,  0, 2, 3,
    6, 5, 4,  7, 6, 4,
    8, 9, 10,  8, 10, 11,
    14, 13, 12,  15, 14, 12,
    16, 17, 18,  16, 18, 19,
    22, 21, 20,  23, 22, 20
  };

  gfx::Buffer ibuffer = renderer.new_buffer(
    gfx::BufferDesc{
      gfx::MAKE_MEMORY(indices),
      gfx::BufferType::INDEX_BUFFER,
    }
    );

  gfx::VertexLayout layout;
  layout.attributes[0].format = gfx::AttributeFormat::FLOAT3;
  layout.attributes[1].format = gfx::AttributeFormat::FLOAT4;
  layout.attributes[2].format = gfx::AttributeFormat::FLOAT2;

  gfx::Pipeline pipe = renderer.new_pipeline(
    gfx::PipelineDesc{
      .shader = shader,
      .layout = layout,
      .index_type = gfx::IndexType::UINT16,
      .cull = gfx::CullMode::FRONT,
    }
    );

  // todo path to asset folder
  core::Image face_img = core::load_image("C:/Users/dheni/source/repos/molten-engine/examples/gfx-textured-cube/awesomeface.png");

  gfx::Texture gfx_face = renderer.new_texture(
    gfx::TextureDesc{
      gfx::Memory(face_img.data),
      gfx::TextureType::TEXTURE_2D,
      gfx::TextureFormat::RGBA8,
      true,
      face_img.width,
      face_img.height
    }
  );

  gfx::TextureDesc target_desc{
    .type = gfx::TextureType::TEXTURE_2D,
    .format = gfx::TextureFormat::RGBA8,
    .width = window_width,
    .height = window_height,
  };

  gfx::Texture color_target = renderer.new_texture(
    target_desc
  );

  target_desc.format = gfx::TextureFormat::DEPTH;

  gfx::Texture depth_target = renderer.new_texture(
    target_desc
  );

  gfx::Bindings bind{
  .vertex_buffer = vbuffer,
  .index_buffer = ibuffer,
  .textures = { color_target, gfx_face },
  };

  gfx::RenderPass offscreen_pass = renderer.new_render_pass(
    gfx::RenderPassDesc{
      .colors = { color_target },
      .depth = depth_target,
    }
    );

  glm::vec2 rotation = glm::vec2(0);
  uint32_t frame_number = 0;

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

    rotation.x += 0.01f;
    rotation.y += 0.03f;
    glm::mat4 model = glm::eulerAngleY(rotation.y) * glm::eulerAngleX(rotation.x);

    glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)window_width / window_height, 0.01f, 10.0f);
    glm::mat4 view = glm::lookAt(
      glm::vec3(0.0f, 1.5f, 3.0f),
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f)
    );

    BasicShader::Uniforms uniforms{
      .mvp = proj * view * model,
    };

    float flash = std::abs(std::sin(frame_number / 120.f));
    renderer.begin_render_pass(
      offscreen_pass,
      gfx::PassAction{
        gfx::ColorAction {
          .color = gfx::Color(0.0, 0.0, flash, 1.0),
        }
      }
    );

    renderer.end_render_pass();

    renderer.begin_default_render_pass(
      gfx::PassAction{
        gfx::ColorAction {
          .color = gfx::Color(0.5, 0.0, 0.0, 1.0),
        }
      }
    );
    renderer.set_pipeline(pipe);
    renderer.set_bindings(bind);
    renderer.set_uniforms(gfx::MAKE_MEMORY(uniforms));
    renderer.draw(0, 36, 1);
    renderer.end_render_pass();

    ++frame_number;

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