#include "shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

namespace core {
  Shader load_shader(const char* path) {
    std::ifstream shader_file;
    // ensure ifstream object can throw exceptions:
    shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    Shader shader;
    try {
      // open files
      shader_file.open(path);
      std::stringstream shader_stream;
      // read file's buffer contents into streams
      shader_stream << shader_file.rdbuf();
      // close file handlers
      shader_file.close();
      // convert stream into string
      shader.code = shader_stream.str();
    }
    catch (std::ifstream::failure e) {
      std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
      return {};
    }

    return shader;
  }

}
