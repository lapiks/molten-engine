#pragma once

#include <string>

namespace core {
  struct Shader {
    std::string code;
  };

  Shader load_shader(const char* path);
}