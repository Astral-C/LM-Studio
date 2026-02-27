#include "USequencer.hpp"
#include "IconsLucide.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <algorithm>
#include <cassert>
#include <fcntl.h>

namespace {
    ImVec2 timelineOrigin { 0, 0 };
    ImVec2 timelineCanvasSize { 0, 0 };
    ImVec2 playheadPosition { 0, 0 };
    bool renderingTimeline { false };
    ImDrawList* list { nullptr };
    float zoom { 10.0f };
    bool litTrack = false;
    ImVec2 viewPosition { 0, 0 };
    ImTimeline::State* currentState { nullptr };
}

void ImTimeline::BeginTimeline(ImTimeline::State* state, bool* playing){
    assert(renderingTimeline == false);
    renderingTimeline = true;
    currentState = state;
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    ImVec2 canvasPosition = ImGui::GetCursorScreenPos();

    list = ImGui::GetWindowDrawList();

    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();


    ImVec2 iconSize = ImGui::CalcTextSize(ICON_LC_PLAY);
    ImVec2 iconPos = canvasPosition+ImVec2(canvasSize.x - style.FramePadding.x - iconSize.x, 0);
    list->AddRectFilled(canvasPosition, canvasPosition+ImVec2(canvasSize.x, iconSize.y + style.FramePadding.y), 0xFF282828);
    if(playing != nullptr){
        list->AddText(iconPos + ImVec2(0,style.FramePadding.y), 0xFFFFFFFF, *playing == false ? ICON_LC_PLAY : ICON_LC_PAUSE);
        if(playing != nullptr && io.MouseClicked[0] && ImRect(iconPos, iconPos+iconSize).Contains(io.MousePos)){
            *playing = !*playing;
        }
    }
    ImGui::Dummy({canvasSize.x, iconSize.y});
    canvasSize = ImGui::GetContentRegionAvail();


    ImGui::BeginChild("##timeline", canvasSize);
    timelineOrigin = ImGui::GetCursorScreenPos();
}

void ImTimeline::EndTimeline(){
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Dummy({1, 2});

    ImVec2 trueSize = ImGui::GetCursorScreenPos() - timelineOrigin;
    if(!currentState->draggingKeyframe && io.MouseDown[0]){
        currentState->currentFrame = (io.MousePos.x - timelineOrigin.x) / zoom;
    }
    ImVec2 playheadTopLeft = timelineOrigin+ImVec2((currentState->currentFrame * zoom)-1, 0);
    ImVec2 playheadBottomRight = timelineOrigin+ImVec2((currentState->currentFrame * zoom)+3,trueSize.y);

    ImGui::GetWindowDrawList()->AddRectFilled(playheadTopLeft, playheadBottomRight, 0x99363636);
    ImGui::EndChild();

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

    ImDrawList* list = list = ImGui::GetWindowDrawList();
    ImVec2 canvasPosition = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    ImGui::Dummy({canvasSize.x, 10});

    list->AddRectFilled(canvasPosition, canvasPosition+ImVec2(canvasSize.x, 10 + (padding.y*2)), 0xFF181818 + (litTrack * 0x00040404), 0);
    canvasPosition.y += 5 + padding.y;

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
