#include "src/include/log.hpp"

#include <print>
#include <stdexcept>

namespace groot {

void Log::generic(const std::string& message) {
  std::print("[Groot Engine] {}\n", message);
}

void Log::runtime_error(const std::string& message) {
  throw std::runtime_error("[Groot Engine] Error: " + message);
}

void Log::out_of_range(const std::string & message) {
  throw std::out_of_range("[Groot Engine] Error:" + message);
}

} // namespace groot