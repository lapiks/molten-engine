#include <glad/glad.h>

#include "renderer.h"

namespace gfx {
  GLenum get_gl_texture_target(TextureType type) {
    switch (type) {
    case TextureType::TEXTURE_1D: return GL_TEXTURE_1D;
    case TextureType::TEXTURE_2D: return GL_TEXTURE_2D;
    }
    return GL_NONE;
  }

  GLenum get_gl_texture_format(TextureFormat format) {
    switch (format) {
    case TextureFormat::RGB: return GL_RGB;
    case TextureFormat::RGBA: return GL_RGBA;
    }
    return GL_NONE;
  }

  GLenum get_gl_primitive_type(PrimitiveType type) {
    switch (type) {
    case PrimitiveType::LINES: return GL_LINES;
    case PrimitiveType::TRIANGLES: return GL_TRIANGLES;
    case PrimitiveType::TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
    case PrimitiveType::TRIANGLE_FAN: return GL_TRIANGLE_FAN;
    }
    return GL_NONE;
  }

  GLenum get_gl_attribute_type(AttributeFormat format) {
    switch (format) {
    case AttributeFormat::FLOAT2: return GL_FLOAT;
    case AttributeFormat::FLOAT3: return GL_FLOAT;
    }
    return GL_NONE;
  }

  uint32_t get_gl_attribute_size(AttributeFormat format) {
    switch (format) {
    case AttributeFormat::FLOAT2: return 2;
    case AttributeFormat::FLOAT3: return 3;
    }
    return 0;
  }

  uint32_t get_gl_type_size(GLenum type) {
    switch (type) {
    case GL_FLOAT: return 32;
    }
    return 0;
  }

  GLenum get_gl_buffer_type(BufferType type) {
    switch (type) {
      case BufferType::VERTEX_BUFFER: return GL_ARRAY_BUFFER;
      case BufferType::INDEX_BUFFER: return GL_ELEMENT_ARRAY_BUFFER;
    }
    return GL_NONE;
  }

  uint16_t gl_size_of_type(UniformType type) {
    switch (type) {
    case UniformType::FLOAT: return 4;
    case UniformType::FLOAT2: return 8;
    case UniformType::FLOAT3: return 12;
    case UniformType::FLOAT4: return 16;
    case UniformType::MAT2: return 16;
    case UniformType::MAT3: return 36;
    case UniformType::MAT4: return 64;
    }
    return 0;
  }
}

