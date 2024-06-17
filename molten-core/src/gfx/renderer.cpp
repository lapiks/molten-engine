#pragma once
#include "gfx/renderer.h"

#include "gl_renderer.h"
#include "vk_renderer.h"

namespace gfx {
#if !(defined(GFX_USE_OPENGL) || defined(GFX_USE_VULKAN))
#error "Please select a backend with GFX_USE_OPENGL or GFX_USE_VULKAN"
#endif

#if defined(GFX_USE_OPENGL)
  static GLRenderer ctx;
#elif defined(GFX_USE_VULKAN)
  static VKRenderer ctx;
#endif

  void Renderer::init(const InitInfo& info) {
    ctx.init(info);
  }

  void Renderer::shutdown() {
    ctx.shutdown();
  }

  void Renderer::draw(uint32_t first_element, uint32_t num_elements, uint32_t num_instances) {
    ctx.draw(first_element, num_elements, num_instances);
  }

  void Renderer::set_viewport(const Rect& rect) {
    ctx.set_viewport(rect);
  }

  void Renderer::set_scissor(const Rect& rect) {
    ctx.set_scissor(rect);
  }

  void Renderer::begin_default_render_pass(const PassAction& action) {
    ctx.begin_render_pass(std::nullopt, action);
  }

  void Renderer::begin_render_pass(RenderPass pass, const PassAction& action) {
    ctx.begin_render_pass(pass, action);
  }

  void Renderer::end_render_pass() {
    ctx.end_render_pass();
  }

  void Renderer::set_pipeline(Pipeline pipe) {
    ctx.set_pipeline(pipe);
  }

  void Renderer::set_bindings(Bindings bind) {
    ctx.set_bindings(bind);
  }

  void Renderer::set_uniforms(const Memory& mem) {
    ctx.set_uniforms(mem);
  }

  void Renderer::submit() {
    ctx.submit();
  }

  Buffer Renderer::new_buffer(const BufferDesc& desc) {
    ctx.new_buffer(_buffer_id, desc);
    return _buffer_id++;
  }

  Texture Renderer::new_texture(const TextureDesc& desc) {
    ctx.new_texture(_texture_id, desc);
    return _texture_id++;
  }
   
  Shader Renderer::new_shader(const ShaderDesc& desc) {
    ctx.new_shader(_shader_id, desc);
    return _shader_id++;
  }

  RenderPass Renderer::new_render_pass(const RenderPassDesc& desc) {
    ctx.new_render_pass(_render_pass_id, desc);
    return _render_pass_id++;
  }

  Pipeline Renderer::new_pipeline(const PipelineDesc& desc) {
    ctx.new_pipeline(_pipeline_id, desc);
    return _pipeline_id++;
  }
}