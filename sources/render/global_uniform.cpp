#include <map>
#include "log.h"
#include "global_uniform.h"
#include "glad/glad.h"

GPUBuffer::GPUBuffer(BufferType type, int bindID, uint initialSize) : bufType(type == BufferType::Storage ? GL_SHADER_STORAGE_BUFFER : GL_UNIFORM_BUFFER), bindID(bindID), bufSize(initialSize)
{
  glGenBuffers(1, &arrayID);
  glBindBuffer(bufType, arrayID);
  glBufferData(bufType, initialSize, NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(bufType, 0);
}

size_t GPUBuffer::size() const
{
  return bufSize;
}

void GPUBuffer::resize_buffer(size_t size)
{
  if (bufSize < size)
  {
    glBufferData(bufType, size, NULL, GL_DYNAMIC_DRAW);
    bufSize = size;
  }
}
void GPUBuffer::update_buffer(const void *data, size_t size) const
{
  glBindBuffer(bufType, arrayID);
  glBindBufferBase(bufType, bindID, arrayID);
  if (bufSize >= size)
  {
  glBufferSubData(bufType, 0, size, data);

  }
  else
    debug_error("buffer size is less than data size % < %", bufSize, size);

  glBindBuffer(bufType, 0);
}

void GPUBuffer::bind() const
{
  glBindBuffer(bufType, arrayID);
  glBindBufferBase(bufType, bindID, arrayID);
}
