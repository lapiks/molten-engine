#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace core {
  struct Transform3D {
    glm::vec3 position = glm::vec3(0);
    glm::quat rotation;
    glm::vec3 scale = glm::vec3(1);
    glm::mat4 model = glm::mat4(1.0);
  };
}