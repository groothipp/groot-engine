#pragma once

#include "src/include/materials.hpp"

#include <vector>

namespace ge {

class Material;

class BufferProxy {
  friend class Material;

  public:
    BufferProxy(unsigned int size) : m_size(size) {}
    BufferProxy(const BufferProxy&) = delete;
    BufferProxy(BufferProxy&&) = delete;

    ~BufferProxy() = default;

    BufferProxy& operator=(const BufferProxy&) = delete;
    BufferProxy& operator=(BufferProxy&&) = delete;

  protected:
    virtual void initialize(void *) const = 0;

  protected:
    const unsigned int m_size;
    Material * m_material = nullptr;
    std::vector<unsigned int> m_offsets;
};

template <typename T>
class Buffer : public BufferProxy {
  public:
    explicit Buffer(const T& data) : BufferProxy(sizeof(T)), m_data(data) {}

    ~Buffer() = default;

    void operator=(const T& data) {
      m_data = data;
      m_material->batchMutable(&m_data, m_size, m_offsets);
    }

    const T& operator*() const {
      return m_data;
    }

  protected:
    void initialize(void * map) const override {
      memcpy(map, &m_data, m_size);
    }

  private:
    T m_data;
};

} // namespace ge