#pragma once

#include "src/include/enums.hpp"

#include <set>
#include <utility>

class GLFWwindow;

namespace groot {

class InputManager {
  using Cursor = std::pair<double, double>;

  std::set<Key> m_heldKeys;
  std::set<Key> m_pressedKeys;
  std::set<Key> m_releasedKeys;

  std::set<MouseButton> m_heldButtons;
  std::set<MouseButton> m_pressedButtons;
  std::set<MouseButton> m_releasedButtons;

  Cursor m_cursor = { 0.0, 0.0 };

  public:
    InputManager() = default;
    InputManager(const InputManager&) = default;
    InputManager(InputManager&&) = default;

    ~InputManager() = default;

    InputManager& operator=(const InputManager&) = default;
    InputManager& operator=(InputManager&&) = default;

    static void keyCallback(GLFWwindow *, int, int, int, int);
    static void cursorCallback(GLFWwindow *, double, double);
    static void mouseCallback(GLFWwindow *, int, int, int);

    bool pressed(Key) const;
    bool pressed(MouseButton) const;
    bool just_pressed(Key) const;
    bool just_pressed(MouseButton) const;
    bool just_released(Key) const;
    bool just_released(MouseButton) const;
    const Cursor& cursor() const;

    void reset();
};

} // namespace groot