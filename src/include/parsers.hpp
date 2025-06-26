#pragma once

#include "src/include/vertex.hpp"

#include <string>
#include <utility>
#include <vector>

namespace ge {

class ObjParser {
  using Output = std::pair<std::vector<Vertex>, std::vector<unsigned int>>;

  public:
    ObjParser() = delete;
    ObjParser(const ObjParser&) = delete;
    ObjParser(ObjParser&&) = delete;

    ~ObjParser() = default;

    ObjParser& operator=(const ObjParser&) = delete;
    ObjParser& operator=(ObjParser&&) = delete;

    static Output parse(const std::string&);
};

class SPVParser {
  using Output = std::vector<char>;

  public:
    SPVParser() = delete;
    SPVParser(const SPVParser&) = delete;
    SPVParser(SPVParser&&) = delete;

    ~SPVParser() = default;

    SPVParser& operator=(const SPVParser&) = delete;
    SPVParser& operator=(SPVParser&&) = delete;

    static Output parse(const std::string&);
};

class PNGParser {
  using Output = std::tuple<
    unsigned int,
    unsigned int,
    unsigned int,
    std::vector<char>
  >;

  public:
    PNGParser() = delete;
    PNGParser(const PNGParser&) = delete;
    PNGParser(PNGParser&&) = delete;

    ~PNGParser() = default;

    PNGParser& operator=(const PNGParser&) = delete;
    PNGParser& operator=(PNGParser&&) = delete;

    static Output parse(const std::string&);
};

} // namespace ge