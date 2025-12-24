#include "src/include/enums.hpp"
#include "src/include/input_mananger.hpp"

#include <GLFW/glfw3.h>

namespace groot {

void InputManager::keyCallback(GLFWwindow * window, int key, int, int action, int) {
  InputManager * inputManager = static_cast<InputManager *>(glfwGetWindowUserPointer(window));

  Key k = static_cast<Key>(key);

  if (action == GLFW_PRESS && !inputManager->pressed(k)) {
    inputManager->m_heldKeys.emplace(k);
    inputManager->m_pressedKeys.emplace(k);
  }
  else if (action == GLFW_RELEASE && inputManager->pressed(k)) {
    inputManager->m_heldKeys.erase(k);
    inputManager->m_releasedKeys.emplace(k);
  }
}

void InputManager::cursorCallback(GLFWwindow * window, double x, double y) {
  InputManager * inputManager = static_cast<InputManager *>(glfwGetWindowUserPointer(window));

  float xScale, yScale;
  glfwGetWindowContentScale(window, &xScale, &yScale);

  inputManager->m_cursor = std::make_pair(x * xScale, y * yScale);
}

void InputManager::mouseCallback(GLFWwindow * window, int button, int action, int) {
  InputManager * inputManager = static_cast<InputManager *>(glfwGetWindowUserPointer(window));

  MouseButton b = static_cast<MouseButton>(button);

  if (action == GLFW_PRESS && !inputManager->pressed(b)) {
    inputManager->m_heldButtons.emplace(b);
    inputManager->m_pressedButtons.emplace(b);
  }
  else if (action == GLFW_RELEASE && inputManager->pressed(b))  {
    inputManager->m_heldButtons.erase(b);
    inputManager->m_releasedButtons.emplace(b);
  }
}

bool InputManager::pressed(Key key) const {
  return m_heldKeys.contains(key);
}

bool InputManager::pressed(MouseButton button) const {
  return m_heldButtons.contains(button);
}

bool InputManager::just_pressed(Key key) const {
  return m_pressedKeys.contains(key);
}

bool InputManager::just_pressed(MouseButton button) const {
  return m_pressedButtons.contains(button);
}

bool InputManager::just_released(Key key) const {
  return m_releasedKeys.contains(key);
}

bool InputManager::just_released(MouseButton button) const {
  return m_releasedButtons.contains(button);
}

const InputManager::Cursor& InputManager::cursor() const {
  return m_cursor;
}

void InputManager::reset() {
  m_pressedKeys.clear();
  m_releasedKeys.clear();
  m_pressedButtons.clear();
  m_releasedButtons.clear();
}

} // namespace groot