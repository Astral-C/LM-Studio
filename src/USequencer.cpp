#include "USequencer.hpp"
#include "IconsLucide.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <algorithm>
#include <cassert>
#include <fcntl.h>

namespace {
    ImVec2 canvasSize { 0, 0 };
    ImVec2 canvasPosition { 0, 0 };
    ImVec2 timelineOrigin { 0, 0 };
    ImVec2 playheadPosition { 0, 0 };
    ImVec2 stickyCrontrolsPos { 0, 0 };
    bool renderingTimeline { false };
    ImDrawList* list { nullptr };
    float zoom { 10.0f };
    bool litTrack = false;
    ImVec2 viewPosition { 0, 0 };
    ImTimeline::State* currentState { nullptr };
}

void ImTimeline::BeginTimeline(ImTimeline::State* state){
    assert(renderingTimeline == false);
    renderingTimeline = true;
    currentState = state;
    canvasSize = ImGui::GetContentRegionAvail();
    canvasPosition = ImGui::GetCursorScreenPos();
    timelineOrigin = canvasPosition;

    list = ImGui::GetWindowDrawList();

    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    playheadPosition = canvasPosition + ImVec2(state->currentFrame, style.FrameBorderSize);

    list->AddRectFilled(canvasPosition, canvasPosition+(canvasSize + ImVec2(style.FrameBorderSize, style.FrameBorderSize)), 0xFF303030, style.FrameRounding);

    canvasPosition += { style.FrameBorderSize, style.FrameBorderSize };
    canvasSize -= { style.FrameBorderSize, style.FrameBorderSize};

    ImGui::PushClipRect(canvasPosition, canvasPosition+canvasSize, false);
    stickyCrontrolsPos = canvasPosition;
    ImVec2 iconSize = ImGui::CalcTextSize(ICON_LC_PLAY);
    canvasPosition.y += style.FramePadding.y + iconSize.y;

    //list->PushClipRect(canvasPosition, canvasPosition+canvasSize);
    if(ImRect(canvasPosition, canvasPosition+canvasSize).Contains(io.MousePos)){
        if(io.KeyShift){
            zoom += io.MouseWheel;
        } else {
            viewPosition.y = std::max(0.0f, viewPosition.y + io.MouseWheel * 5);

            if(state->trackCount != -1){
                viewPosition.y = std::min(viewPosition.y, (12 * state->trackCount) - (canvasSize.y / 10));
            }
        }
    }
    canvasPosition -= viewPosition;
}

void ImTimeline::EndTimeline(){
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();

    if(!currentState->draggingKeyframe && io.MouseDown[0]){
        currentState->currentFrame = (io.MousePos.x - timelineOrigin.x) / zoom;
    }

    list->AddRectFilled(playheadPosition+ImVec2((currentState->currentFrame * zoom)-1,0), playheadPosition+ImVec2((currentState->currentFrame * zoom)+3, canvasSize.y), 0xAA323232);

    ImVec2 iconSize = ImGui::CalcTextSize(ICON_LC_PLAY);
    list->AddRectFilled(stickyCrontrolsPos, stickyCrontrolsPos+ImVec2(canvasSize.x, iconSize.y + style.FramePadding.y), 0xFF282828);
    list->AddText(stickyCrontrolsPos+ImVec2(canvasSize.x - style.FramePadding.x - iconSize.x, style.FramePadding.y), 0xFFFFFFFF, ICON_LC_PLAY);

    list->PopClipRect();
    canvasSize = { 0, 0 };
    canvasPosition = { 0, 0 };
    renderingTimeline = false;
    list = nullptr;
    litTrack = false;
    currentState = nullptr;
}

bool ImTimeline::RenderTrack(std::vector<LKeyframeCommon> &keyframes, int keyframeScale){
    bool edited { false };
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 padding = style.FramePadding;

    list->AddRectFilled(canvasPosition, canvasPosition+ImVec2(canvasSize.x, 10 + (padding.y*2)), 0xFF181818 + (litTrack * 0x00040404), 0);
    canvasPosition.y += 05 + padding.y;

    for(auto& keyframe : keyframes){
        ImVec2 keyframePos = { canvasPosition.x + (keyframe.frame * zoom), canvasPosition.y };
        list->AddCircleFilled(keyframePos, 5, 0xFF656D3F );

        ImVec2 dist = (io.MousePos - keyframePos);

        if(fabs(dist.x) < 5 && fabs(dist.y) < 5 && io.MouseClicked[0]){
            std::cout << "clicked keyframe " << keyframe.frame << std::endl;
            currentState->draggingKeyframe = true;
            currentState->selectedKeyframe = &keyframe;
            currentState->draggingKeyframeStart = keyframe.frame;
        }
    }

    if(io.MouseDown[0] && currentState->draggingKeyframe && currentState->selectedKeyframe != nullptr){
        currentState->selectedKeyframe->frame = (io.MousePos.x - timelineOrigin.x) / zoom;
    }

    if(io.MouseReleased[0] && currentState->draggingKeyframe && currentState->selectedKeyframe != nullptr){
        currentState->draggingKeyframe = false;
        currentState->selectedKeyframe = nullptr;
        currentState->draggingKeyframeStart = 0.0f;
        edited = true;
    }

    canvasPosition.y += 5 + padding.y;

    litTrack = !litTrack;
    return edited;
}
