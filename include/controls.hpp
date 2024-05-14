#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#include "./use_opengl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void updateMovement(GLFWwindow *window);
glm::vec2 getRotation();
glm::vec3 getPosition();

#endif
