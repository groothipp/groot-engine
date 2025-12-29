#include "src/include/gui.hpp"
#include "src/include/log.hpp"

#include <cstring>
#include <imgui.h>

#include <format>
#include <memory>

namespace groot {

GUI::Element::ReturnValue GUI::Element::value() {
  return 0.0f;
}

GUI::Builder& GUI::Builder::text(const std::string& txt) {
  m_elements.emplace(std::format("text_{}", m_texts++), std::make_unique<Text>(txt));
  return *this;
}

GUI::Builder& GUI::Builder::separator() {
  m_elements.emplace(std::format("separator_{}", m_separators++), std::make_unique<Separator>());
  return *this;
}

GUI::Builder& GUI::Builder::button(const std::string& label, const std::function<void()>& callback) {
  if (m_elements.contains(label))
    Log::runtime_error(std::format("Duplicate GUI elements -- {}", label));

  m_elements.emplace(label, std::make_unique<Button>(label, callback));

  return *this;
}

GUI::Builder& GUI::Builder::toggle(const std::string& label, bool start_state) {
  if (m_elements.contains(label))
    Log::runtime_error(std::format("Duplicate GUI elements -- {}", label));

  m_elements.emplace(label, std::make_unique<Toggle>(label, start_state));

  return *this;
}

template <typename T>
GUI::Builder& GUI::Builder::slider(const std::string& label, T min, T max, T start_val) {
  static_assert(
    std::is_same_v<T, int> || std::is_same_v<T, float>,
    "Slider type must be of type int or float"
  );

  if (m_elements.contains(label))
    Log::runtime_error(std::format("Duplicate GUI elements -- {}", label));

  start_val = std::clamp(start_val, min, max);
  m_elements.emplace(label, std::make_unique<Slider<T>>(label, min, max, start_val));

  return *this;
}

template <typename T>
GUI::Builder& GUI::Builder::input(const std::string& label, T start_val) {
  static_assert(
    std::is_same_v<T, float>        ||
    std::is_same_v<T, int>          ||
    std::is_same_v<T, std::string>  ||
    std::is_same_v<T, vec2>         ||
    std::is_same_v<T, vec3>         ||
    std::is_same_v<T, vec4>,
    "Input must be of type float, int, string, vec2, vec3, or vec4"
  );

  if (m_elements.contains(label))
    Log::runtime_error(std::format("Duplicate GUI elements for tag \"{}\"", label));

  m_elements.emplace(label, std::make_unique<Input<T>>(label, start_val));

  return *this;
}

GUI::Builder& GUI::Builder::sub_gui(const std::string& label, GUI&& gui, bool group) {
  if (m_elements.contains(label))
    Log::runtime_error(std::format("Duplicate GUI elements -- {}", label));

  if (group)
    m_elements.emplace(label, std::make_unique<Group>(label, std::move(gui)));
  else
    m_elements.emplace(label, std::make_unique<SubGUI>(std::move(gui)));

  return *this;
}

GUI::Builder& GUI::Builder::custom(const std::string& label, std::unique_ptr<GUI::Element>&& element) {
  if (m_elements.contains(label))
    Log::runtime_error(std::format("Duplicate GUI elements -- {}", label));

  m_elements.emplace(label, std::move(element));

  return *this;
}

GUI GUI::Builder::build() {
  return GUI(std::move(m_elements));
}

GUI::GUI(std::unordered_map<std::string, std::unique_ptr<Element>>&& elements) : m_elements(std::move(elements)) {}

void GUI::toggle() {
  m_active = !m_active;
}

void GUI::render() {
  if (!m_active) return;

  for (auto& [key, element] : m_elements)
    element->render();
}

Text::Text(const std::string& text) : m_text(text) {}

void Text::render() {
  ImGui::Text("%s", m_text.c_str());
}

void Separator::render() {
  ImGui::Separator();
}

Button::Button(const std::string& label, const std::function<void()>& callback) : m_label(label), m_callback(callback) {}

void Button::render() {
  if (ImGui::Button(m_label.c_str()))
    m_callback();
}

Toggle::Toggle(const std::string& label, bool start_state) : m_label(label), m_state(start_state) {}

GUI::Element::ReturnValue Toggle::value() {
  return m_state;
}

void Toggle::render() {
  ImGui::Checkbox(m_label.c_str(), &m_state);
}

template <typename T>
Slider<T>::Slider(const std::string& label, T min, T max, T start_val)
: m_label(label), m_min(min), m_max(max), m_val(start_val)
{}

template <typename T>
GUI::Element::ReturnValue Slider<T>::value() {
  return m_val;
}

template <typename T>
void Slider<T>::render() {
  if constexpr (std::is_same_v<T, float>) {
    ImGui::SliderFloat(m_label.c_str(), &m_val, m_min, m_max);
  }
  else if constexpr (std::is_same_v<T, int>) {
    ImGui::SliderInt(m_label.c_str(), &m_val, m_min, m_max);
  }
}

template <typename T>
Input<T>::Input(const std::string& label, T start_val) : m_label(label), m_val(start_val) {}

template <typename T>
GUI::Element::ReturnValue Input<T>::value() {
  return m_val;
}

template <typename T>
void Input<T>::render() {
  if constexpr (std::is_same_v<T, float>) {
    ImGui::InputFloat(m_label.c_str(), &m_val);
  }
  else if constexpr (std::is_same_v<T, int>) {
    ImGui::InputInt(m_label.c_str(), &m_val);
  }
  else if constexpr (std::is_same_v<T, std::string>) {
    static const unsigned int maxChars = 1024;

    char buffer[maxChars];
    std::strncpy(buffer, m_val.c_str(), maxChars - 1);
    buffer[maxChars - 1] = '\0';

    if (ImGui::InputText(m_label.c_str(), buffer, maxChars))
      m_val = buffer;
  }
  else if constexpr (std::is_same_v<T, vec2>) {
    ImGui::InputFloat2(m_label.c_str(), reinterpret_cast<float *>(&m_val));
  }
  else if constexpr (std::is_same_v<T, vec3>) {
    ImGui::InputFloat3(m_label.c_str(), reinterpret_cast<float *>(&m_val));
  }
  else if constexpr (std::is_same_v<T, vec4>) {
    ImGui::InputFloat4(m_label.c_str(), reinterpret_cast<float *>(&m_val));
  }
}

Group::Group(const std::string& label, GUI&& gui) : m_label(label), m_gui(std::move(gui)) {}

void Group::render() {
  if (ImGui::CollapsingHeader(m_label.c_str())) {
    ImGui::Indent();
    m_gui.render();
    ImGui::Unindent();
  }
}

SubGUI::SubGUI(GUI&& gui) : m_gui(std::move(gui)) {}

void SubGUI::render() {
  m_gui.render();
}

template class Slider<int>;
template class Slider<float>;

template class Input<int>;
template class Input<float>;
template class Input<std::string>;
template class Input<vec2>;
template class Input<vec3>;
template class Input<vec4>;

template GUI::Builder& GUI::Builder::slider<int>(const std::string&, int, int, int);
template GUI::Builder& GUI::Builder::slider<float>(const std::string&, float, float, float);

template GUI::Builder& GUI::Builder::input<int>(const std::string&, int);
template GUI::Builder& GUI::Builder::input<float>(const std::string&, float);
template GUI::Builder& GUI::Builder::input<std::string>(const std::string&, std::string);
template GUI::Builder& GUI::Builder::input<vec2>(const std::string&, vec2);
template GUI::Builder& GUI::Builder::input<vec3>(const std::string&, vec3);
template GUI::Builder& GUI::Builder::input<vec4>(const std::string&, vec4);

} // namespace groot