#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <3dmath.h>
#include "ozz/base/maths/simd_math.h"


struct Mesh
{
  const uint32_t vertexArrayBufferObject;
  const int numIndices;

  std::vector<ozz::math::Float4x4> invBindPose;
  std::vector<ozz::math::Float4x4> bindPose;


  Mesh(uint32_t vertexArrayBufferObject, int numIndices) :
    vertexArrayBufferObject(vertexArrayBufferObject),
    numIndices(numIndices)
    {}
};

using MeshPtr = std::shared_ptr<Mesh>;

MeshPtr make_plane_mesh();
MeshPtr make_mesh(const std::vector<uint32_t> &indices, const std::vector<vec3> &vertices, const std::vector<vec3> &normals);

void render(const MeshPtr &mesh);
void render(const MeshPtr &mesh, int count);