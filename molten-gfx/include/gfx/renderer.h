#pragma once

#include <stdint.h>

namespace gfx {
  using BufferHandle = uint32_t;
  using TextureHandle = uint32_t;
  using ShaderHandle = uint32_t;
  using PassHandle = uint32_t;
  using PipelineHandle = uint32_t;

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
    uint32_t size = 0;
  };

  struct BufferDesc {
    Memory* mem = nullptr;
  };

  struct TextureDesc {
    Memory* mem = nullptr;
    TextureType type = TextureType::TEXTURE_2D;
    TextureFormat format = TextureFormat::RGB;
    uint32_t width = 0;
    uint32_t height = 0;
  };

  struct PipelineCommon {
    PrimitiveType primitive_type = PrimitiveType::TRIANGLES;
  };

  struct ShaderDesc {
    Memory* vertex_mem = nullptr;
    Memory* fragment_mem = nullptr;
  };

  class Renderer {
  public:
    void init(void* glProcAdress);
    void draw(uint32_t first_element, uint32_t num_elements);

    BufferHandle new_buffer(const BufferDesc& desc);
    TextureHandle new_texture(const TextureDesc& desc);
    ShaderHandle new_shader(const ShaderDesc& desc);
    PassHandle new_pass();
    PipelineHandle new_pipeline();

  private:
    uint32_t _buffer_id = 0;
    uint32_t _texture_id = 0;
    uint32_t _shader_id = 0;
    uint32_t _pass_id = 0;
    uint32_t _pipeline_id = 0;
  };
}
