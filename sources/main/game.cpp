
#include <render/direction_light.h>
#include <render/material.h>
#include <render/scene.h>
#include "camera.h"
#include <application.h>
#include <render/debug_arrow.h>
#include <imgui/imgui.h>
#include "ImGuizmo.h"

#include "ozz/animation/runtime/animation.h"
#include "ozz/animation/runtime/local_to_model_job.h"
#include "ozz/animation/runtime/sampling_job.h"
#include "ozz/animation/runtime/skeleton.h"
#include "ozz/base/maths/simd_math.h"
#include "ozz/base/maths/soa_transform.h"

struct UserCamera
{
  glm::mat4 transform;
  mat4x4 projection;
  ArcballCamera arcballCamera;
};

struct Character
{
  glm::mat4 transform;
  MeshPtr mesh;
  MaterialPtr material;
};

struct Scene
{
  DirectionLight light;

  UserCamera userCamera;

  std::vector<Character> characters;
};

static std::unique_ptr<Scene> scene;
static std::vector<std::string> animationList;

#include <filesystem>
static std::vector<std::string> scan_animations(const char *path)
{
  std::vector<std::string> animations;
  for (auto &p : std::filesystem::recursive_directory_iterator(path))
  {
    auto filePath = p.path();
    if (p.is_regular_file() && filePath.extension() == ".fbx")
      animations.push_back(filePath.string());
  }
  return animations;
}

void game_init()
{
  animationList = scan_animations("resources/Animations");
  scene = std::make_unique<Scene>();
  scene->light.lightDirection = glm::normalize(glm::vec3(-1, -1, 0));
  scene->light.lightColor = glm::vec3(1.f);
  scene->light.ambient = glm::vec3(0.2f);

  scene->userCamera.projection = glm::perspective(90.f * DegToRad, get_aspect_ratio(), 0.01f, 500.f);

  ArcballCamera &cam = scene->userCamera.arcballCamera;
  cam.curZoom = cam.targetZoom = 0.5f;
  cam.maxdistance = 5.f;
  cam.distance = cam.curZoom * cam.maxdistance;
  cam.lerpStrength = 10.f;
  cam.mouseSensitivity = 0.5f;
  cam.wheelSensitivity = 0.05f;
  cam.targetPosition = glm::vec3(0.f, 1.f, 0.f);
  cam.targetRotation = cam.curRotation = glm::vec2(DegToRad * -90.f, DegToRad * -30.f);
  cam.rotationEnable = false;

  scene->userCamera.transform = calculate_transform(scene->userCamera.arcballCamera);

  input.onMouseButtonEvent += [](const SDL_MouseButtonEvent &e)
  { arccam_mouse_click_handler(e, scene->userCamera.arcballCamera); };
  input.onMouseMotionEvent += [](const SDL_MouseMotionEvent &e)
  { arccam_mouse_move_handler(e, scene->userCamera.arcballCamera); };
  input.onMouseWheelEvent += [](const SDL_MouseWheelEvent &e)
  { arccam_mouse_wheel_handler(e, scene->userCamera.arcballCamera); };

  auto material = make_material("character", "sources/shaders/character_vs.glsl", "sources/shaders/character_ps.glsl");
  std::fflush(stdout);
  material->set_property("mainTex", create_texture2d("resources/MotusMan_v55/MCG_diff.jpg"));

  SceneAsset sceneAsset = load_scene("resources/MotusMan_v55/MotusMan_v55.fbx",
                                     SceneAsset::LoadScene::Meshes | SceneAsset::LoadScene::Skeleton);

  scene->characters.emplace_back(Character{
                                     glm::identity<glm::mat4>(),
                                     sceneAsset.meshes[0],
                                     std::move(material)});

  create_arrow_render();

  std::fflush(stdout);
}

void game_update()
{
  arcball_camera_update(
      scene->userCamera.arcballCamera,
      scene->userCamera.transform,
      get_delta_time());
}

void render_character(const Character &character, const mat4 &cameraProjView, vec3 cameraPosition, const DirectionLight &light)
{
  const Material &material = *character.material;
  const Shader &shader = material.get_shader();

  shader.use();
  material.bind_uniforms_to_shader();
  shader.set_mat4x4("Transform", character.transform);
  shader.set_mat4x4("ViewProjection", cameraProjView);
  shader.set_vec3("CameraPosition", cameraPosition);
  shader.set_vec3("LightDirection", glm::normalize(light.lightDirection));
  shader.set_vec3("AmbientLight", light.ambient);
  shader.set_vec3("SunLight", light.lightColor);

  size_t boneNumber = character.mesh->bindPose.size();
  std::vector<mat4> bones(boneNumber);

  // const RuntimeSkeleton &skeleton = character.skeleton;
  // size_t nodeCount = skeleton.ref->nodeCount;
  // for (size_t i = 0; i < nodeCount; i++)
  // {
  //   auto it = character.mesh->nodeToBoneMap.find(skeleton.ref->names[i]);
  //   if (it != character.mesh->nodeToBoneMap.end())
  //   {
  //     int boneIdx = it->second;
  //     bones[boneIdx] = skeleton.globalTm[i] * character.mesh->invBindPose[boneIdx];
  //   }
  // }
  shader.set_mat4x4("Bones", bones);

  render(character.mesh);

  // for (size_t i = 0; i < nodeCount; i++)
  // {
  //   glm::vec3 offset{0,0,0};
  //   for (size_t j = i; j < nodeCount; j++)
  //   {
  //     if (skeleton.ref->parent[j] == int(i))
  //     {
  //       offset = glm::vec3(skeleton.localTm[j][3]);
  //       draw_arrow(skeleton.globalTm[i], vec3(0), offset, vec3(0, 0.5f, 0), 0.01f);
  //     }
  //   }
  // }
}

void render_imguizmo(ImGuizmo::OPERATION &mCurrentGizmoOperation, ImGuizmo::MODE &mCurrentGizmoMode)
{
  if (ImGui::Begin("gizmo window"))
  {
    if (ImGui::IsKeyPressed('Z'))
      mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed('E'))
      mCurrentGizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed('R')) // r Key
      mCurrentGizmoOperation = ImGuizmo::SCALE;
    if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
      mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
      mCurrentGizmoOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
      mCurrentGizmoOperation = ImGuizmo::SCALE;

    if (mCurrentGizmoOperation != ImGuizmo::SCALE)
    {
      if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
        mCurrentGizmoMode = ImGuizmo::LOCAL;
      ImGui::SameLine();
      if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
        mCurrentGizmoMode = ImGuizmo::WORLD;
    }
  }
  ImGui::End();
}

void imgui_render()
{
  ImGuizmo::BeginFrame();
  for (Character &character : scene->characters)
  {
    // const auto &skeleton = *character.skeleton;
    // size_t nodeCount = skeleton.num_joints();

    // if (ImGui::Begin("Skeleton view"))
    // {
    //   for (size_t i = 0; i < nodeCount; i++)
    //   {
    //     ImGui::Text("%d) %s", int(i), skeleton.joint_names()[i]);
    //   }
    // }
    // ImGui::End();

    if (ImGui::Begin("Animation list"))
    {
      std::vector<const char *> animations(animationList.size() + 1);
      animations[0] = "None";
      for (size_t i = 0; i < animationList.size(); i++)
        animations[i + 1] = animationList[i].c_str();
      static int item = 0;
      if (ImGui::Combo(animations[item], &item, animations.data(), animations.size()))
      {
        // AnimationPtr animation;
        // if (item > 0)
        // {
        //   SceneAsset sceneAsset = load_scene(animations[item],
        //                                      SceneAsset::LoadScene::Skeleton | SceneAsset::LoadScene::Animation);
        //   if (!sceneAsset.animations.empty())
        //     animation = sceneAsset.animations[0];
        // }
        // character.currentAnimation = animation;
        // character.animTime = 0;
      }
    }
    ImGui::End();

    static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
    render_imguizmo(mCurrentGizmoOperation, mCurrentGizmoMode);

    const glm::mat4 &projection = scene->userCamera.projection;
    const glm::mat4 &transform = scene->userCamera.transform;
    mat4 cameraView = inverse(transform);
    ImGuiIO &io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    glm::mat4 globNodeTm = character.transform;

    ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(projection), mCurrentGizmoOperation, mCurrentGizmoMode,
                         glm::value_ptr(globNodeTm));

    character.transform = globNodeTm;

    break;
  }
}

void game_render()
{
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  const float grayColor = 0.3f;
  glClearColor(grayColor, grayColor, grayColor, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const glm::mat4 &projection = scene->userCamera.projection;
  const glm::mat4 &transform = scene->userCamera.transform;
  glm::mat4 projView = projection * inverse(transform);

  for (const Character &character : scene->characters)
    render_character(character, projView, glm::vec3(transform[3]), scene->light);

  render_arrows(projView, glm::vec3(transform[3]), scene->light);
}

void close_game()
{
  scene.reset();
}