#include <glad/glad.h>

#include "renderer.h"

namespace gfx {
  GLuint get_gl_texture_target(TextureType type) {
    switch (type) {
    case TextureType::TEXTURE_1D: return GL_TEXTURE_1D;
    case TextureType::TEXTURE_2D: return GL_TEXTURE_2D;
    }

    return GL_NONE;
  }

  GLuint get_gl_texture_format(TextureFormat format) {
    switch (format) {
    case TextureFormat::RGB: return GL_RGB;
    case TextureFormat::RGBA: return GL_RGBA;
    }

    return GL_NONE;
  }

  GLuint get_gl_primitive_type(PrimitiveType type) {
    switch (type) {
    case PrimitiveType::LINES: return GL_LINES;
    case PrimitiveType::TRIANGLES: return GL_TRIANGLES;
    case PrimitiveType::TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
    case PrimitiveType::TRIANGLE_FAN: return GL_TRIANGLE_FAN;
    }

    return GL_NONE;
  }
}

