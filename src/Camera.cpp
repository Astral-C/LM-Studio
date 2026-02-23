#include "Camera.hpp"
#include "UInput.hpp"
#include "imgui.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <algorithm>
#include <GLFW/glfw3.h>

UCamera::UCamera() : mNearPlane(1.0f), mFarPlane(1000000.f), mFovy(glm::radians(60.f)), mAspectRatio(16.f / 9.f), mDistance(0.0f)
{
    mProjection = glm::perspective(mFovy, mAspectRatio, mNearPlane, mFarPlane);
    mView = glm::lookAt(glm::vec3(800.0f, 800.0f, 800.0f),glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
    mDistance = glm::distance(glm::vec3(0.0f,0.0f,0.0f), glm::vec3(800.0f, 800.0f, 800.0f));
}

void UCamera::Update(float deltaTime){
    if(ImGui::GetIO().MouseWheel != 0.0f){
        float amt = ImGui::GetIO().MouseWheel * 15.0f;
        glm::vec3 forward = ZERO - glm::vec3(glm::inverse(mView)[3]);
        mView = glm::translate(mView, glm::normalize(forward) * amt);
        mDistance = glm::distance(glm::vec3(0,0,0), glm::vec3(glm::inverse(mView)[3]));
    }
    if(UInput::GetMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT) && UInput::GetMouseDelta() != glm::vec2(0.0f, 0.0f)){
        glm::vec2 delta = UInput::GetMouseDelta() * 20.f;
        glm::vec3 curUp = glm::normalize(glm::cross(glm::vec3(mView[1]), glm::vec3(mView[2])));
        mView = glm::translate(mView, glm::normalize(glm::vec3(mView[1])) * delta.x);
        mView = glm::translate(mView, curUp * delta.y);
    }
}
