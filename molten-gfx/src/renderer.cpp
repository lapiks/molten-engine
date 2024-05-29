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
}