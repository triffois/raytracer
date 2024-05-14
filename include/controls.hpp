#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#include "./use_opengl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum {
    MODE_MOUSE = 0,
    MODE_ARROWS = 1,
};

void update_movement(GLFWwindow *window, int mode);
glm::vec2 get_rotation();
glm::vec3 get_position();

#endif
