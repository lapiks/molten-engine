#pragma once

#include <stdint.h>

namespace gfx {
  using BufferHandle = uint32_t;
  using TextureHandle = uint32_t;
  using ShaderHandle = uint32_t;
  using ProgramHandle = uint32_t;
  using PassHandle = uint32_t;
  using PipelineHandle = uint32_t;

  class Renderer {
  public:
    void init(void* glProcAdress);
    void draw();

    BufferHandle new_buffer();
    TextureHandle new_texture();
    ShaderHandle new_shader();
    ProgramHandle new_program();
    PassHandle new_pass();
    PipelineHandle new_pipeline();

  private:
    uint32_t _buffer_id = 0;
    uint32_t _texture_id = 0;
    uint32_t _shader_id = 0;
    uint32_t _program_id = 0;
    uint32_t _pass_id = 0;
    uint32_t _pipeline_id = 0;
  };
}
