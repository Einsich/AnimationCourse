#pragma once
#include <vector>
using uint = unsigned int;

enum class BufferType
{
  Uniform,
  Storage
};

struct GPUBuffer
{
private:
  uint arrayID;
  uint bufType;
  int bindID;
  uint bufSize;
public:
  GPUBuffer() = default;
  GPUBuffer(BufferType type, int bindID, uint initialSize);

  size_t size() const;
  void resize_buffer(size_t size);
  void update_buffer(const void *data, size_t size) const;
  void bind() const;
};
