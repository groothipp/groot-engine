#pragma once

#include "src/include/linalg.hpp"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

namespace groot {

class GUI {
  friend class Group;
  friend class Renderer;
  friend class SubGUI;

  public:
    class Element {
      friend class GUI;

      protected:
        using ReturnValue = std::variant<
          float,
          int,
          std::string,
          bool,
          vec2,
          vec3,
          vec4
        >;

      public:
        virtual ~Element() = default;
        virtual ReturnValue value();

      protected:
        virtual void render() = 0;
    };

    class Builder {
      std::unordered_map<std::string, std::unique_ptr<Element>> m_elements;
      unsigned int m_separators = 0;
      unsigned int m_texts = 0;

      public:
        Builder() = default;

        Builder& text(const std::string&);
        Builder& separator();
        Builder& button(const std::string&, const std::function<void()>&);
        Builder& toggle(const std::string&, bool start_state = false);

        template <typename T = float>
        Builder& slider(const std::string&, T, T, T start_val = T{});

        template <typename T = float>
        Builder& input(const std::string&, T start_val = T{});

        Builder& sub_gui(const std::string&, GUI&&, bool group = false);
        Builder& custom(const std::string&, std::unique_ptr<GUI::Element>&&);

        GUI build();
    };

  private:
    bool m_active = true;
    std::unordered_map<std::string, std::unique_ptr<Element>> m_elements;

  public:
    GUI() = delete;
    GUI(const GUI&) = delete;
    GUI(GUI&&) = default;

    GUI& operator=(const GUI&) = delete;
    GUI& operator=(GUI&&) = default;

    template <typename T>
    inline std::unique_ptr<T>& get_element(const std::string& label) {
      static_assert(std::is_base_of_v<GUI::Element, T>, "T must inherit from GUI::Element");

      std::unique_ptr<GUI::Element>& element = m_elements.at(label);

      T* casted = dynamic_cast<T*>(element.get());
      if (casted == nullptr) Log::bad_cast();

      return reinterpret_cast<std::unique_ptr<T>&>(element);
    }

    void toggle();

  private:
    explicit GUI(std::unordered_map<std::string, std::unique_ptr<Element>>&&);
    void render();
};

class Text : public GUI::Element {
  std::string m_text = "";

  public:
    Text(const std::string&);

  protected:
    void render() override;
};

class Separator : public GUI::Element {
  public:
    Separator() = default;

  protected:
    void render() override;
};

class Button : public GUI::Element {
  std::string m_label = "";
  std::function<void()> m_callback;

  public:
    Button(const std::string&, const std::function<void()>&);

  protected:
    void render() override;
};

class Toggle : public GUI::Element {
  std::string m_label = "";
  bool m_state = false;

  public:
    Toggle(const std::string&, bool start_state);
    ReturnValue value() override;

  protected:
    void render() override;
};

template <typename T = float>
class Slider : public GUI::Element {
  static_assert(
    std::is_same_v<T, int> || std::is_same_v<T, float>,
    "Slider type must be of type int or float"
  );

  std::string m_label = "";
  T m_val = T{};
  T m_min = T{};
  T m_max = T{};

  public:
    Slider(const std::string&, T, T, T start_val = T{});
    ReturnValue value() override;

  protected:
    void render() override;
};

template <typename T>
class Input : public GUI::Element {
  static_assert(
    std::is_same_v<T, float>        ||
    std::is_same_v<T, int>          ||
    std::is_same_v<T, std::string>  ||
    std::is_same_v<T, vec2>         ||
    std::is_same_v<T, vec3>         ||
    std::is_same_v<T, vec4>,
    "Input must be of type float, int, string, vec2, vec3, or vec4"
  );

  std::string m_label = "";
  T m_val = T{};

  public:
    explicit Input(const std::string&, T start_val = T{});
    ReturnValue value() override;

  protected:
    void render() override;
};

class Group : public GUI::Element {
  std::string m_label = "";
  GUI m_gui;

  public:
    Group(const std::string&, GUI&&);

    template<typename T>
    inline std::unique_ptr<T>& get_element(const std::string& label) {
      return m_gui.get_element<T>(label);
    };

  protected:
    void render() override;
};

class SubGUI : public GUI::Element {
  GUI m_gui;

  public:
    explicit SubGUI(GUI&&);

    template<typename T>
    inline std::unique_ptr<T>& get_element(const std::string& label) {
      return m_gui.get_element<T>(label);
    }

  protected:
    void render() override;
};

} // namespace groot