#pragma once

#include <stdint.h>

namespace gfx {
  constexpr size_t MAX_VERTEX_BUFFERS = 8;

  using Buffer = uint32_t;
  using Texture = uint32_t;
  using Shader = uint32_t;
  using Pass = uint32_t;
  using Pipeline = uint32_t;

  enum class ShaderType {
    VERTEX,
    FRAGMENT,
  };

  enum class TextureType {
    TEXTURE_1D,
    TEXTURE_2D,
  };

  enum class TextureFormat {
    RGB,
    RGBA,
  };

  enum class PrimitiveType {
    LINES,
    TRIANGLES,
    TRIANGLE_FAN,
    TRIANGLE_STRIP,
  };

  struct Memory {
    void* data = nullptr;
    size_t size = 0;
  };

#define MAKE_MEMORY(x) Memory { &x, sizeof(x) }

  struct BufferDesc {
    Memory mem;
  };

  struct Bindings {
    Buffer vertex_buffers[MAX_VERTEX_BUFFERS];
  };

  struct TextureDesc {
    Memory mem;
    TextureType type = TextureType::TEXTURE_2D;
    TextureFormat format = TextureFormat::RGB;
    uint32_t width = 0;
    uint32_t height = 0;
  };

  struct PipelineCommon {
    PrimitiveType primitive_type = PrimitiveType::TRIANGLES;
  };

  struct ShaderDesc {
    const char* vertex_src = nullptr;
    const char* fragment_src = nullptr;
  };

  struct PipelineDesc {
    Shader shader;
  };

  class Renderer {
  public:
    void init(void* glProcAdress);
    void apply_pipeline(Pipeline pipe);
    void apply_bindings(Bindings bind);
    void draw(uint32_t first_element, uint32_t num_elements);

    Buffer new_buffer(const BufferDesc& desc);
    Texture new_texture(const TextureDesc& desc);
    Shader new_shader(const ShaderDesc& desc);
    Pass new_pass();
    Pipeline new_pipeline(const PipelineDesc& desc);

  private:
    uint32_t _buffer_id = 0;
    uint32_t _texture_id = 0;
    uint32_t _shader_id = 0;
    uint32_t _pass_id = 0;
    uint32_t _pipeline_id = 0;
  };
}
