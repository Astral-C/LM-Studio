#include "Camera.hpp"
#include "UInput.hpp"
#include "imgui.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <algorithm>
#include <format>
#include <GLFW/glfw3.h>

UCamera::UCamera() : mNearPlane(1.0f), mFarPlane(1000000.f), mFovy(glm::radians(60.f)), mAspectRatio(16.f / 9.f), mDistance(0.0f)
{
    mProjection = glm::perspective(mFovy, mAspectRatio, mNearPlane, mFarPlane);
    mView = glm::lookAt(glm::vec3(800.0f, 800.0f, 800.0f),glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
    mDistance = glm::distance(glm::vec3(0.0f,0.0f,0.0f), glm::vec3(800.0f, 800.0f, 800.0f));
}

void UCamera::SetOrbitPoint(glm::vec3 orbitPoint, glm::vec3 dist){
    mCenter = orbitPoint;
    mView = glm::lookAt(orbitPoint + dist, orbitPoint, glm::vec3(0.0f,1.0f,0.0f));
    mDistance = glm::distance(orbitPoint, orbitPoint + dist);
}

void UCamera::UpdateSize(ImVec2 size){
    mProjection = glm::perspective(mFovy, size.x/size.y, mNearPlane, mFarPlane);
}

void UCamera::Update(float deltaTime){
    if(ImGui::GetIO().MouseWheel != 0.0f){
        float amt = ImGui::GetIO().MouseWheel * 15.0f;
        glm::vec3 forward = glm::vec3(glm::inverse(mView)[3]) - mCenter;
        mView = glm::translate(mView, glm::normalize(forward) * amt);
        mDistance = glm::distance(mCenter, glm::vec3(glm::inverse(mView)[3]));
    }

    glm::vec2 mouseDelta = UInput::GetMouseDelta();
    if(ImGui::IsMouseDragging(ImGuiMouseButton_Middle) && mouseDelta != glm::vec2(0.0f, 0.0f)){
        glm::vec2 delta = {mouseDelta.x, mouseDelta.y};
        glm::vec3 forward = glm::vec3(glm::inverse(mView)[3]) - mCenter;
        glm::vec3 right = glm::normalize(glm::cross(forward, UNIT_Y));
        glm::vec3 up = glm::normalize(glm::cross(forward, right));
        mView = glm::translate(mView, right * -delta.x);
        mView = glm::translate(mView, up * delta.y);
    }

    if(ImGui::IsMouseDragging(ImGuiMouseButton_Right) && mouseDelta != glm::vec2(0.0f, 0.0f)){
        glm::vec2 delta = {mouseDelta.x, mouseDelta.y};
        mView = glm::rotate(mView, glm::radians(-delta.x), glm::vec3(0,1,0));
        mView = glm::rotate(mView, glm::radians(delta.y), glm::vec3(1,0,0));
    }
}
