#pragma once

#include "deferred_voxel_renderer.h"
#include "shapes.h"
#include "shader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace core {
  struct GBufferPass {
    static GPURenderPass create(gfx::Renderer& renderer, uint32_t width, uint32_t height) {
      gfx::TextureDesc target_desc{
        .type = gfx::TextureType::TEXTURE_2D,
        .format = gfx::TextureFormat::RGB8,
        .width = width,
        .height = height,
      };

      gfx::Texture pos_target = renderer.new_texture(
        target_desc
      );

      gfx::Texture normal_target = renderer.new_texture(
        target_desc
      );

      gfx::Texture color_target = renderer.new_texture(
        target_desc
      );

      std::vector<gfx::Texture> targets = { pos_target, normal_target, color_target };

      gfx::RenderPass gbuffer_pass = renderer.new_render_pass(
        gfx::RenderPassDesc{
          .colors = targets,
        }
      );

      return GPURenderPass{
        .targets = targets,
        .rpass = gbuffer_pass,
      };
    }
  };

  struct GBufferPipeline {
    struct Uniforms {
      glm::mat4 model;
      glm::mat4 view_proj;
    };

    static GPUPipeline create(gfx::Renderer& renderer) {
      Shader vs = core::load_shader("assets/shaders/gbuffer.vert");
      Shader fs = core::load_shader("assets/shaders/gbuffer.frag");

      gfx::ShaderDesc desc{
        .vertex_src = vs.code.c_str(),
        .fragment_src = fs.code.c_str(),
        .uniforms_layout = gfx::UniformBlockLayout {
          .uniforms = {
            gfx::UniformDesc {
              .name = "u_model",
              .type = gfx::UniformType::MAT4,
            },
            gfx::UniformDesc {
              .name = "u_view_proj",
              .type = gfx::UniformType::MAT4,
            }
          },
        },
        .texture_names = { "u_tex" },
      };

      gfx::Shader shader = renderer.new_shader(desc);

      gfx::VertexLayout layout;
      layout.attributes[0].format = gfx::AttributeFormat::FLOAT3;

      gfx::Pipeline pip = renderer.new_pipeline(
        gfx::PipelineDesc{
          .shader = shader,
          .layout = layout,
          .index_type = gfx::IndexType::UINT16,
          .cull = gfx::CullMode::BACK,
        }
      );

      return GPUPipeline{
        .shader = shader,
        .pipeline = pip,
      };
    }
  };

  struct ScreenQuadPipeline {
    static GPUPipeline create(gfx::Renderer& renderer) {
      Shader vs = core::load_shader("assets/shaders/screen_quad.vert");
      Shader fs = core::load_shader("assets/shaders/screen_quad.frag");

      gfx::ShaderDesc desc{
        .vertex_src = vs.code.c_str(),
        .fragment_src = fs.code.c_str(),
        .texture_names = { "u_tex" },
      };

      gfx::Shader shader = renderer.new_shader(desc);

      gfx::VertexLayout layout;
      layout.attributes[0].format = gfx::AttributeFormat::FLOAT3;
      layout.attributes[1].format = gfx::AttributeFormat::FLOAT2;

      gfx::Pipeline pip = renderer.new_pipeline(
        gfx::PipelineDesc{
          .shader = shader,
          .layout = layout,
          .index_type = gfx::IndexType::UINT16,
          .cull = gfx::CullMode::BACK,
        }
        );

      return GPUPipeline{
        .shader = shader,
        .pipeline = pip,
      };
    }
  };

  struct Cube {
    static GPUMesh create(gfx::Renderer& renderer) {
      gfx::Buffer vbuffer = renderer.new_buffer(
        gfx::BufferDesc{
          gfx::MAKE_MEMORY(shapes::cube::VERTICES),
          gfx::BufferType::VERTEX_BUFFER,
        }
      );

      gfx::Buffer ibuffer = renderer.new_buffer(
        gfx::BufferDesc{
          gfx::MAKE_MEMORY(shapes::cube::INDICES),
          gfx::BufferType::INDEX_BUFFER,
        }
      );

      return GPUMesh{
        vbuffer,
        ibuffer,
      };
    }
  };

  struct Quad {
    static GPUMesh create(gfx::Renderer& renderer) {
      gfx::Buffer vbuffer = renderer.new_buffer(
        gfx::BufferDesc{
          gfx::MAKE_MEMORY(shapes::quad::VERTICES),
          gfx::BufferType::VERTEX_BUFFER,
        }
        );

      return GPUMesh{
        .vbuffer = vbuffer,
        .ibuffer = std::nullopt,
      };
    }
  };

  void DeferredVoxelRenderer::init(const gfx::InitInfo& info) {
    _renderer.init(info);

    // create pipelines
    _gbuffer_pip = GBufferPipeline::create(_renderer);
    _screen_quad_pip = ScreenQuadPipeline::create(_renderer);

    // create render pass
    _gbuffer_pass = GBufferPass::create(_renderer, 800, 600);

    // create meshes
    _cube = Cube::create(_renderer);
    _quad = Quad::create(_renderer);

    // bindings
    _cube_bind = {
      .vertex_buffer = _cube.vbuffer,
      .index_buffer = _cube.ibuffer,
    };

    _quad_bind = {
      .vertex_buffer = _quad.vbuffer,
      .textures = { _gbuffer_pass.targets[1] },
    };
  }

  void DeferredVoxelRenderer::render() {
    glm::mat4 model(1.0f);
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)800.0f / 600.0f, 0.01f, 10.0f);
    glm::mat4 view = glm::lookAt(
      glm::vec3(0.0f, 1.5f, 3.0f),
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f)
    );

    GBufferPipeline::Uniforms uniforms{
      .model = model,
      .view_proj = proj * view,
    };

    _renderer.begin_render_pass(
      _gbuffer_pass.rpass,
      gfx::PassAction{
        gfx::ColorAction {
          .color = gfx::Color(0.1f, 0.1f, 0.1f, 1.0f),
        }
      }
    );

    _renderer.set_pipeline(_gbuffer_pip.pipeline);
    _renderer.set_bindings(_cube_bind);
    _renderer.set_uniforms(gfx::MAKE_MEMORY(uniforms));
    _renderer.draw(0, 36, 1);
    _renderer.end_render_pass();

    _renderer.begin_default_render_pass(
      gfx::PassAction{
        gfx::ColorAction {
          .color = gfx::Color(0.0f, 0.0f, 0.0f, 1.0f),
        }
      }
    );
    _renderer.set_pipeline(_screen_quad_pip.pipeline);
    _renderer.set_bindings(_quad_bind);
    _renderer.draw(0, 4, 1);
    _renderer.end_render_pass();

    _renderer.submit();
  }

  void DeferredVoxelRenderer::shutdown() {
    _renderer.shutdown();
  }
}