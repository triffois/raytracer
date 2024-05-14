// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "./controls.hpp"

glm::vec3 position = glm::vec3(0, 0, -5);

float yaw = glm::pi<float>();
float pitch = 0.0f;
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.005f;

glm::vec2 getRotation() { return glm::vec2(pitch, yaw); }
glm::vec3 getPosition() { return position; }

void updateMovement(GLFWwindow *window) {

  // Get window size
  int width, height;
  glfwGetWindowSize(window, &width, &height);

  // glfwGetTime is called only once, the first time this function is called
  static double lastTime = glfwGetTime();

  // Compute time difference between current and last frame
  double currentTime = glfwGetTime();
  float deltaTime = float(currentTime - lastTime);

  // Get mouse position
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);

  // Reset mouse position for next frame
  glfwSetCursorPos(window, width / 2.0, height / 2.0);

  // Compute new orientation
  yaw += mouseSpeed * (width / 2.0 - xpos);
  pitch += mouseSpeed * (height / 2.0 - ypos);

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
    position += direction * deltaTime * speed;
  }
  // Move backward
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    position -= direction * deltaTime * speed;
  }
  // Strafe right
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    position += right * deltaTime * speed;
  }
  // Strafe left
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    position -= right * deltaTime * speed;
  }
  // Move up
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    position += glm::vec3(0, 1, 0) * deltaTime * speed;
  }
  // Move down
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
    position -= glm::vec3(0, 1, 0) * deltaTime * speed;
  }

  // Zoom in/out with mouse wheel
  double yoffset = 0;
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
    double prevY;
    glfwGetCursorPos(window, nullptr, &prevY);
    glfwPollEvents();
    double currY;
    glfwGetCursorPos(window, nullptr, &currY);
    yoffset = currY - prevY;
  }
  float FoV = initialFoV - 5 * static_cast<float>(yoffset);

  // For the next frame, the "last time" will be "now"
  lastTime = currentTime;
}
