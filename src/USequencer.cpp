#include "USequencer.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <cassert>

namespace {
    ImVec2 canvasSize { 0, 0 };
    ImVec2 canvasPosition { 0, 0 };
    bool renderingTimeline { false };
    ImDrawList* list { nullptr };
    float zoom { 10.0f };
    bool litTrack = false;
    ImVec2 viewPosition { 0, 0 };
}

void ImTimeline::BeginTimeline(){
    assert(renderingTimeline == false);
    canvasSize = ImGui::GetContentRegionAvail();
    canvasPosition = ImGui::GetCursorScreenPos();

    list = ImGui::GetWindowDrawList();

    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    list->AddRectFilled(canvasPosition, canvasPosition+canvasSize, 0xFF252525, 0);
    canvasPosition += { style.FrameBorderSize, style.FrameBorderSize };
    canvasSize -= { style.FrameBorderSize, style.FrameBorderSize};
    ImGui::PushClipRect(canvasPosition, canvasPosition+canvasSize, false);
    //list->PushClipRect(canvasPosition, canvasPosition+canvasSize);
    if(ImRect(canvasPosition, canvasPosition+canvasSize).Contains(io.MousePos)){
        if(io.KeyShift){
            zoom += io.MouseWheel;
        } else {
            viewPosition.y = std::max(0.0f, viewPosition.y + io.MouseWheel * 5);
        }
    }
    canvasPosition -= viewPosition;
}

void ImTimeline::EndTimeline(){
    list->PopClipRect();
    canvasSize = { 0, 0 };
    canvasPosition = { 0, 0 };
    renderingTimeline = false;
    list = nullptr;
    litTrack = false;
}

void ImTimeline::RenderTrack(std::vector<LKeyframeCommon> &keyframes, int keyframeScale){
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 padding = style.FramePadding;

    list->AddRectFilled(canvasPosition, canvasPosition+ImVec2(canvasSize.x, 15 + (padding.y*2)), 0xFF181818 + (litTrack * 0x00040404), 0);

    canvasPosition.y += 05 + padding.y;

    for(auto& keyframe : keyframes){
        list->AddCircleFilled({ canvasPosition.x + (keyframe.frame * zoom), canvasPosition.y }, 5, 0xFF656D3F );
    }

    canvasPosition.y += 5 + padding.y;

    litTrack = !litTrack;
}
