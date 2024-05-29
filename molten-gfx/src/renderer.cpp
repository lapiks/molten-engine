#include "renderer.h"
#pragma once

#include "gl_renderer.h"

namespace gfx {
  static GLRenderer ctx;

  void Renderer::init(void* glProcAdress) {
    ctx.init(glProcAdress);
  }

  void Renderer::draw(uint32_t first_element, uint32_t num_elements) {
    ctx.draw(first_element, num_elements);
  }

  BufferHandle Renderer::new_buffer(const BufferDesc& desc) {
    ctx.new_buffer(_buffer_id, desc);
    return _buffer_id++;
  }

  TextureHandle Renderer::new_texture(const TextureDesc& desc) {
    ctx.new_texture(_texture_id, desc);
    return _texture_id++;
  }
   
  ShaderHandle Renderer::new_shader(const ShaderDesc& desc) {
    ctx.new_shader(_shader_id, desc);
    return _shader_id++;
  }

  PassHandle Renderer::new_pass() {
    ctx.new_pass(_pass_id);
    return _pass_id++;
  }

  PipelineHandle Renderer::new_pipeline() {
    ctx.new_pipeline(_pipeline_id);
    return _pipeline_id++;
  }
}