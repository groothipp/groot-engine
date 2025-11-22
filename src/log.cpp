#include "src/include/log.hpp"

#include <print>
#include <stdexcept>

namespace groot {

void Log::generic(const std::string& message) {
  std::print("[Groot Engine] {}\n", message);
}

void Log::warn(const std::string& message) {
  std::print("\033[33m[Groot Engine] {}\033[0m\n", message);
}

void Log::runtime_error(const std::string& message) {
  throw std::runtime_error("\033[31m[Groot Engine] " + message + "\033[0m");
}

void Log::out_of_range(const std::string & message) {
  throw std::out_of_range("\033[31m[Groot Engine] " + message + "\033[0m");
}

} // namespace groot