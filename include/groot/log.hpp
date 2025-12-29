#pragma once

#include <string>

namespace groot {

class Log {
  public:
    Log() = delete;
    Log(const Log&) = delete;
    Log(Log&&) = delete;

    ~Log() = default;

    Log& operator=(const Log&) = delete;
    Log& operator=(Log&&) = delete;

    static void generic(const std::string&);
    static void warn(const std::string&);
    static void runtime_error(const std::string&);
    static void out_of_range(const std::string&);
    static void bad_cast();
};

} // namespace groot