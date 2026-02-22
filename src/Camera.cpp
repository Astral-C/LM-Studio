#include "Camera.hpp"
#include "UInput.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <algorithm>
#include <GLFW/glfw3.h>

UCamera::UCamera() : mNearPlane(1.0f), mFarPlane(1000000.f), mFovy(glm::radians(60.f)), mAspectRatio(16.f / 9.f),
    mMoveSpeed(1000.f), mMouseSensitivity(0.25f), mWinWidth(1280), mWinHeight(720), mDistance(100.0f) {

    mProjection = glm::perspective(mFovy, mAspectRatio, mNearPlane, mFarPlane);
    mView = glm::lookAt(glm::vec3(-800, 800, 800), glm::vec3(0,0,0), glm::vec3(0,1,0));
    mDistance = glm::distance(glm::vec3(-800,800,800), glm::vec3(0,0,0));
}

void UCamera::Update(float deltaTime) {
	glm::vec3 moveDir = glm::zero<glm::vec3>();

	mMoveSpeed += UInput::GetMouseScrollDelta() * 100 * deltaTime;
	mMoveSpeed = std::clamp(mMoveSpeed, 100.f, 50000.f);
	float actualMoveSpeed = UInput::GetKey(GLFW_KEY_LEFT_SHIFT) ? mMoveSpeed * 10.f : mMoveSpeed;

	if (UInput::GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT)) Rotate(deltaTime, UInput::GetMouseDelta());
}

void UCamera::Rotate(float deltaTime, glm::vec2 mouseDelta) {
	if (mouseDelta.x == 0.f && mouseDelta.y == 0.f)
		return;


}
