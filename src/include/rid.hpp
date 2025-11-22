#pragma once

namespace groot {

class RID {
  friend class Engine;

  unsigned long m_id = ~(0x0);

  public:
    RID() = default;
    RID(const RID&) = default;
    RID(RID&&) = default;

    ~RID() = default;

    RID& operator=(const RID&) = default;
    RID& operator=(RID&&) = default;

    const unsigned long& operator*() const;

    bool is_valid() const;

  private:
    explicit RID(unsigned long);

    void invalidate();
};

} // namespace groot