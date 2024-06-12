#pragma once

#include <stdint.h>
#include <vector>

#define GFX_USE_OPENGL
//#define GFX_USE_VULKAN

struct SDL_Window;

namespace gfx {
  // CONSTANTS
  constexpr size_t MAX_VERTEX_BUFFERS = 8;
  constexpr size_t MAX_ATTRIBUTES = 16;
  constexpr size_t MAX_PASSES = 4096;

  // DEFINES
  using Buffer = uint32_t;
  using Texture = uint32_t;
  using Shader = uint32_t;
  using Pass = uint32_t;
  using Pipeline = uint32_t;

  // ENUMS
  enum class ShaderStage {
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

  enum class AttributeFormat {
    FLOAT2,
    FLOAT3,
  };

  enum class BufferType {
    VERTEX_BUFFER,
    INDEX_BUFFER,
  };

  enum class IndexType {
    NONE,
    UINT16,
    UINT32,
  };

  enum class UniformType {
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    MAT2,
    MAT3,
    MAT4,
  };

  enum class Action {
    NOTHING,
    CLEAR,
  };

  // STRUCTS
  struct InitInfo {
    SDL_Window* window = nullptr;
  };

  struct Memory {
    void* data = nullptr;
    size_t size = 0;
  };

#define MAKE_MEMORY(x) Memory { &x, sizeof(x) }

  struct Color {
    float r, g, b, a = 0;

    Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
  };

  struct ColorAction {
    Action action = Action::CLEAR;
    Color color;
  };

  struct DepthAction {
    Action action = Action::CLEAR;
    float value = 1.0f;
  };

  struct StencilAction {
    Action action = Action::CLEAR;
    float value = 0.0f;
  };

  struct PassAction {
    ColorAction color_action;
    DepthAction depth_action;
    StencilAction stencil_action;
  };

  struct PassData {

  };

  struct PipelineCommon {
    PrimitiveType primitive_type = PrimitiveType::TRIANGLES;
  };

  struct Bindings {
    Buffer vertex_buffers[MAX_VERTEX_BUFFERS];
    Buffer index_buffer;
  };

  struct VertexAttribute {
    int32_t index = 0;
    size_t stride = 0;
    AttributeFormat format = AttributeFormat::FLOAT3;
  };

  struct VertexLayout {
    VertexAttribute attributes[MAX_ATTRIBUTES];
  };

  struct UniformDesc {
    const char* name;
    UniformType type;
  };

  struct UniformBlockLayout {
    std::vector<UniformDesc> uniforms;
  };

  // OBJECT CREATION STRUCTS
  struct BufferDesc {
    Memory mem;
    BufferType type;
  };

  struct TextureDesc {
    Memory mem;
    TextureType type = TextureType::TEXTURE_2D;
    TextureFormat format = TextureFormat::RGB;
    uint32_t width = 0;
    uint32_t height = 0;
  };

  struct ShaderDesc {
    const char* vertex_src = nullptr;
    const char* fragment_src = nullptr;
    UniformBlockLayout uniforms_layout;
  };

  struct PipelineDesc {
    Shader shader;
    VertexLayout layout;
    IndexType index_type;
  };

  struct PassDesc {

  };

  struct Rect {
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t width = 0;
    uint16_t height = 0;

    Rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) : x(x), y(y), width(width), height(height) {}
  };

  /*!
  * Backend agnostic Renderer
  */
  class Renderer {
  public:
    void init(const InitInfo& info);
    void shutdown();
    void begin_pass(Pass pass, const PassAction& action);
    void begin_default_pass(const PassAction& action);
    void set_pipeline(Pipeline pipe);
    void set_bindings(Bindings bind);
    void set_uniforms(ShaderStage stage, const Memory& mem);
    void draw(uint32_t first_element, uint32_t num_elements, uint32_t num_instances);
    void set_viewport(const Rect& rect);
    void set_scissor(const Rect& rect);

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

    PassData _passes[MAX_PASSES];
  };
}
