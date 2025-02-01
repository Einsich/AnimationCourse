#pragma once

#include "mesh.h"
#include "ozz/base/maths/simd_math.h"


namespace ozz
{
  namespace animation
  {
    class Skeleton;
    class Animation;
  }
}

struct Skeleton
{
  std::shared_ptr<ozz::animation::Skeleton> skeleton;
  std::vector<ozz::math::Float4x4> invBindPose;
  std::vector<ozz::math::Float4x4> bindPose;
};

using SkeletonPtr = std::shared_ptr<Skeleton>;
using AnimationPtr = std::shared_ptr<ozz::animation::Animation>;

struct SceneAsset
{
  std::vector<MeshPtr> meshes;
  SkeletonPtr skeleton;
  std::vector<AnimationPtr> animations;
  enum LoadScene
  {
    Meshes = 1 << 0,
    Skeleton = 1 << 1,
    Animation = 1 << 2,
    AdditiveAnimation = 1 << 3,
  };
};

SceneAsset load_scene(const char *path, int load_flags, SkeletonPtr ref_pos = nullptr);