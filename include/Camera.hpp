#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "ImGuizmo.h"

struct GLFWwindow;

constexpr glm::vec3 ZERO = glm::vec3(0.0f, 0.0f, 0.0f);
constexpr glm::vec3 UNIT_X = glm::vec3(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 UNIT_Y = glm::vec3(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 UNIT_Z = glm::vec3(0.0f, 0.0f, 1.0f);

constexpr float LOOK_UP_MIN = -glm::half_pi<float>() + glm::epsilon<float>();
constexpr float LOOK_UP_MAX = glm::half_pi<float>() - glm::epsilon<float>();

class UCamera {

	float mNearPlane;
	float mFarPlane;
	float mFovy;
	float mAspectRatio;

	float mMoveSpeed;
	float mMouseSensitivity;
	int mWinWidth, mWinHeight;

	void Rotate(float deltaTime, glm::vec2 mouseDelta);

public:
	UCamera();
	~UCamera() {}

	float mDistance;
	void Update(float deltaTime);

	glm::mat4 mProjection, mView;

};
