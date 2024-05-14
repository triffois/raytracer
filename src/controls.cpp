// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
using namespace glm;

#include "./controls.hpp"

glm::vec3 position = glm::vec3(0, 0, -5);

float yaw = glm::pi<float>();
float pitch = 0.0f;
float initial_fov = 45.0f;

float speed = 3.0f; // 3 units / second
float mouse_speed = 0.005f;

glm::vec2 get_rotation() { return glm::vec2(pitch, yaw); }
glm::vec3 get_position() { return position; }

void update_movement(GLFWwindow *window, int mode) {

    // Get window size
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // glfwGetTime is called only once, the first time this function is called
    static double lastTime = glfwGetTime();

    // Compute time difference between current and last frame
    double current_time = glfwGetTime();
    float delta_time = float(current_time - lastTime);

    switch (mode) {
    case MODE_MOUSE:
        std::cout << "Mouse mode" << std::endl;
        // Get mouse position
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Reset mouse position for next frame
        glfwSetCursorPos(window, width / 2.0, height / 2.0);

        // Compute new orientation
        yaw += mouse_speed * (width / 2.0 - xpos);
        pitch += mouse_speed * (height / 2.0 - ypos);

        break;
    case MODE_ARROWS:
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            pitch += mouse_speed * delta_time * 100;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            pitch -= mouse_speed * delta_time * 100;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            yaw += mouse_speed * delta_time * 100;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            yaw -= mouse_speed * delta_time * 100;
        }
    }

    // Clamp pitch
    if (pitch > glm::pi<float>() / 2.0f) {
        pitch = glm::pi<float>() / 2.0f;
    }
    if (pitch < -glm::pi<float>() / 2.0f) {
        pitch = -glm::pi<float>() / 2.0f;
    }

    // Compute purely horizontal direction
    glm::vec3 direction(-sin(yaw), 0, -cos(yaw));
    glm::vec3 right(-sin(yaw - glm::pi<float>() / 2.0f), 0,
                    -cos(yaw - glm::pi<float>() / 2.0f));

    // Move forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        position += direction * delta_time * speed;
    }
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        position -= direction * delta_time * speed;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        position += right * delta_time * speed;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        position -= right * delta_time * speed;
    }
    // Move up
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        position += glm::vec3(0, 1, 0) * delta_time * speed;
    }
    // Move down
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        position -= glm::vec3(0, 1, 0) * delta_time * speed;
    }

    // Zoom in/out with mouse wheel
    double yoffset = 0;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
        double prev_y;
        glfwGetCursorPos(window, nullptr, &prev_y);
        glfwPollEvents();
        double curr_y;
        glfwGetCursorPos(window, nullptr, &curr_y);
        yoffset = curr_y - prev_y;
    }
    float fov = initial_fov - 5 * static_cast<float>(yoffset);

    // For the next frame, the "last time" will be "now"
    lastTime = current_time;
}
