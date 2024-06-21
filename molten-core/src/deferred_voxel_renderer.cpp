#pragma once

#include "deferred_voxel_renderer.h"
#include "shapes.h"
#include "shader.h"

// todo remove
#include "vox_scene.h"
#include "ogt_vox.h"

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
      glm::mat4 view;
      glm::mat4 proj;
      glm::vec3 model_dim;
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
              .name = "u_view",
              .type = gfx::UniformType::MAT4,
            },
            gfx::UniformDesc {
              .name = "u_proj",
              .type = gfx::UniformType::MAT4,
            },
            gfx::UniformDesc {
              .name = "u_model_dim",
              .type = gfx::UniformType::FLOAT3,
            },
          },
        },
        .texture_names = { "u_vox_model" },
      };

      gfx::Shader shader = renderer.new_shader(desc);

      gfx::VertexLayout layout;
      layout.attributes[0].format = gfx::AttributeFormat::FLOAT3;
      layout.attributes[1].format = gfx::AttributeFormat::FLOAT4;
      layout.attributes[2].format = gfx::AttributeFormat::FLOAT2;
      layout.attributes[3].format = gfx::AttributeFormat::FLOAT3;

      gfx::Pipeline pip = renderer.new_pipeline(
        gfx::PipelineDesc{
          .shader = shader,
          .layout = layout,
          .index_type = gfx::IndexType::UINT16,
          .primitive_type = gfx::PrimitiveType::TRIANGLE_STRIP,
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
          .index_type = gfx::IndexType::NONE,
          .primitive_type = gfx::PrimitiveType::TRIANGLE_STRIP,
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
        .vbuffer = vbuffer,
        .ibuffer = ibuffer,
        .vertex_count = sizeof(shapes::cube::VERTICES),
        .index_count = sizeof(shapes::cube::INDICES),
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
        .vertex_count = sizeof(shapes::quad::VERTICES),
        .index_count = 0,
      };
    }
  };

  void DeferredVoxelRenderer::init(const gfx::InitInfo& info) {
    _renderer.init(info);

    // create pipelines
    _gbuffer_pip = GBufferPipeline::create(_renderer);
    _screen_quad_pip = ScreenQuadPipeline::create(_renderer);

    // create render pass
    _gbuffer_pass = GBufferPass::create(_renderer, 1024, 680);

    // create meshes
    _cube = Cube::create(_renderer);
    _quad = Quad::create(_renderer);

    // todo: remove
    VoxScene vox_scene;
    vox_scene.load("assets/models/chr_knight.vox");
    const ogt_vox_model* model = vox_scene.ogt_scene->models[0];
    model_dim = {
      model->size_x,
      model->size_y,
      model->size_z,
    };

    vox_texture = _renderer.new_texture(
      gfx::TextureDesc{
        .mem = (uint8_t*)model->voxel_data,
        .type = gfx::TextureType::TEXTURE_3D,
        .format = gfx::TextureFormat::R8,
        .width = model->size_x,
        .height = model->size_y,
        .depth = model->size_z,
      }
    );

    // bindings
    _cube_bind = {
      .vertex_buffer = _cube.vbuffer,
      .index_buffer = _cube.ibuffer,
      .textures = { vox_texture },
    };

    _quad_bind = {
      .vertex_buffer = _quad.vbuffer,
      .textures = { _gbuffer_pass.targets[2] },
    };
  }

  void DeferredVoxelRenderer::render() {
    rotation.x += 0.01f;
    rotation.y += 0.03f;
    glm::mat4 model = glm::eulerAngleY(rotation.y) * glm::eulerAngleX(rotation.x);
    //glm::mat4 model = glm::mat4(1.0);
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)1024.0f / 680.0f, 0.01f, 10.0f);
    glm::mat4 view = glm::lookAt(
      glm::vec3(0.0f, 0.0f, 2.0f),
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f)
    );

    GBufferPipeline::Uniforms uniforms{
      .model = model,
      .view = view,
      .proj = proj,
      .model_dim = model_dim
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
    _renderer.draw(0, 14, 1);
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