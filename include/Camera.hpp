#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "ImGuizmo.h"
#include "glm/fwd.hpp"

struct GLFWwindow;

constexpr glm::vec3 ZERO = glm::vec3(0.0f, 0.0f, 0.0f);
constexpr glm::vec3 UNIT_X = glm::vec3(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 UNIT_Y = glm::vec3(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 UNIT_Z = glm::vec3(0.0f, 0.0f, 1.0f);

constexpr float LOOK_UP_MIN = -glm::half_pi<float>() + glm::epsilon<float>();
constexpr float LOOK_UP_MAX = glm::half_pi<float>() - glm::epsilon<float>();

class UCamera {

	float mAspectRatio;
	float mNearPlane;
	float mFarPlane;
	float mFovy;

public:
	UCamera();
	~UCamera() {}

	float mDistance;
	glm::vec3 mCenter { 0.0f };
	glm::mat4 mProjection { glm::mat4(1.0f) }, mView { glm::mat4(1.0f) };

    void SetOrbitPoint(glm::vec3 orbitPoint, glm::vec3 dist=glm::vec3(800,800,800));
	void Update(float deltaTime);
	void UpdateSize(ImVec2 size);
};
