#include "src/include/log.hpp"

#include <iostream>
#include <print>
#include <stdexcept>

namespace groot {

void Log::generic(const std::string& message) {
  std::println(std::cout, "[Groot Engine] {}", message);
}

void Log::warn(const std::string& message) {
  std::println(std::cout, "\033[33m[Groot Engine] {}\033[0m", message);
}

void Log::runtime_error(const std::string& message) {
  throw std::runtime_error(std::format("\033[31m\n[Groot Engine] {}\n\033[0m", message));
}

void Log::out_of_range(const std::string& message) {
  throw std::out_of_range(std::format("\033[31m\n[Groot Engine] {}\n\033[0m", message));
}

void Log::bad_cast() {
  throw std::bad_cast();
}

} // namespace groot