#include "renderer.h"
#pragma once

#include "gl_renderer.h"

namespace gfx {
  static GLRenderer ctx;

  void Renderer::init(void* glProcAdress) {
    ctx.init(glProcAdress);
  }

  void Renderer::draw() {
    ctx.draw();
  }

  BufferHandle Renderer::new_buffer() {
    ctx.new_buffer(_buffer_id);
    return _buffer_id++;
  }

  TextureHandle Renderer::new_texture() {
    ctx.new_texture(_texture_id);
    return _texture_id++;
  }
   
  ShaderHandle Renderer::new_shader() {
    ctx.new_shader(_shader_id);
    return _shader_id++;
  }

  ProgramHandle Renderer::new_program() {
    ctx.new_program(_program_id);
    return _program_id++;
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