#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#include "./use_opengl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void computeMatricesFromInputs(GLFWwindow *window);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

#endif
