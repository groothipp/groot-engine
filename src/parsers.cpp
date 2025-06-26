#include "src/include/parsers.hpp"

#include <fstream>
#include <map>
#include <png.h>
#include <sstream>
#include <stdexcept>
#include <tuple>

namespace ge {

ObjParser::Output ObjParser::parse(const std::string& path) {
  std::ifstream file(path);
  if (!file) throw std::runtime_error("groot-engine: failed to open " + path);

  std::vector<vec3> positions;
  std::vector<vec2> uvs;
  std::vector<vec3> normals;

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  std::map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int> indexMap;

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream ss(line);

    std::string key;
    ss >> key;

    if (key == "v") {
      vec3 position = vec3(0.0f);
      ss >> position.x >> position.y >> position.z;
      positions.emplace_back(position);
    }
    else if (key == "vt") {
      vec2 uv = vec2(0.0f);
      ss >> uv.x >> uv.y;
      uvs.emplace_back(uv);
    }
    else if (key == "vn") {
      vec3 normal = vec3(0.0f);
      ss >> normal.x >> normal.y >> normal.z;
      normals.emplace_back(normal);
    }
    else if (key == "f") {
      std::string token;
      while (ss >> token) {
        std::replace(token.begin(), token.end(), '/', ' ');
        std::istringstream ts(token);

        unsigned int vertIndex, uvIndex, normIndex;
        ts >> vertIndex >> uvIndex >> normIndex;

        if (indexMap.contains({ vertIndex, uvIndex, normIndex })) {
          indices.emplace_back(indexMap.at({ vertIndex, uvIndex, normIndex }));
          continue;
        }

        indexMap.emplace(std::make_pair(std::tuple{ vertIndex, uvIndex, normIndex }, vertices.size()));
        indices.emplace_back(vertices.size());
        vertices.emplace_back(Vertex{ positions[vertIndex - 1], uvs[uvIndex - 1], normals[normIndex - 1] });
      }
    }
  }

  return { std::move(vertices), std::move(indices) };
}

SPVParser::Output SPVParser::parse(const std::string& path) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file) throw std::runtime_error("groot-engine: failed to open shader at " + path);

  std::vector<char> buffer;
  unsigned int size = file.tellg();

  if (size < 4)
    throw std::runtime_error("groot-engine: file too small - " + path);

  if (size % 4 != 0)
    throw std::runtime_error("groot-engine: corrupted shader - " + path);

  buffer.resize(4);
  file.seekg(0);
  file.read(buffer.data(), 4);

  if (*reinterpret_cast<unsigned int *>(buffer.data()) != 0x07230203)
    throw std::runtime_error("groot-engine: incorrect file format - " + path);

  buffer.resize(size);
  file.read(buffer.data() + 4, size - 4);

  return buffer;
}

PNGParser::Output PNGParser::parse(const std::string& path) {
  FILE * fp = fopen(path.c_str(), "rb");
  if (!fp) throw std::runtime_error("groot-engine: failed to read file at " + path);

  png_byte header[8];
  fread(header, 1, 8, fp);
  if (png_sig_cmp(header, 0, 8)) {
    fclose(fp);
    throw std::runtime_error("groot-engine: " + path + " is not a PNG");
  }

  png_structp p_png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  png_infop p_info = png_create_info_struct(p_png);
  if (!(p_png && p_info)) {
    fclose(fp);
    throw std::runtime_error("groot-engine: failed to initialize libpng");
  }

  if (setjmp(png_jmpbuf(p_png))) {
    fclose(fp);
    png_destroy_read_struct(&p_png, &p_info, nullptr);
    throw std::runtime_error("groot-engine: failed to read PNG at " + path);
  }

  png_init_io(p_png, fp);
  png_set_sig_bytes(p_png, 8);
  png_read_info(p_png, p_info);

  int width = png_get_image_width(p_png, p_info);
  int height = png_get_image_height(p_png, p_info);
  png_byte colorType = png_get_color_type(p_png, p_info);
  png_byte bitDepth = png_get_bit_depth(p_png, p_info);

  if (bitDepth == 16)
    png_set_strip_16(p_png);

  if (colorType == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(p_png);
  else if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
    png_set_expand_gray_1_2_4_to_8(p_png);

  if (png_get_valid(p_png, p_info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(p_png);

  if (colorType == PNG_COLOR_TYPE_RGB || colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(p_png, 0xFF, PNG_FILLER_AFTER);

  if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(p_png);

  png_read_update_info(p_png, p_info);

  png_bytepp p_rows = (png_bytepp)malloc(sizeof(png_bytep) * height);
  int rowSize = png_get_rowbytes(p_png, p_info);
  for (int row = 0; row < height; ++row)
    p_rows[row] = (png_bytep)malloc(rowSize);

  png_read_image(p_png, p_rows);

  std::vector<char> image(height * rowSize);
  for (int row = 0; row < height; ++row) {
    memcpy(image.data() + row * rowSize, p_rows[row], rowSize);
    free(p_rows[row]);
  }
  free(p_rows);

  fclose(fp);
  png_destroy_read_struct(&p_png, &p_info, nullptr);

  return { width, height, height * rowSize, std::move(image) };
}

} // namespace ge