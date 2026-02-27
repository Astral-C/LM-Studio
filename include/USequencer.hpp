#ifndef __SEQUENCER_H__
#define __SEQUENCER_H__
#include "imgui.h"
#include "imgui_internal.h"
#include "io/KeyframeIO.hpp"
#include <vector>

namespace ImTimeline {

    struct State {
      float currentFrame { 0 };
      LKeyframeCommon* selectedKeyframe { nullptr };

      bool draggingKeyframe { false };
      float draggingKeyframeStart { 0.0f };
      int trackCount { -1 };
    };

    void BeginTimeline(ImTimeline::State* state, bool* playing = nullptr);
    void EndTimeline();

    bool RenderTrack(std::vector<LKeyframeCommon>& keyframes, int keyframeScale=5);
};
#endif
