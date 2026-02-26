#include "imgui.h"
#include "imgui_internal.h"
#include "io/KeyframeIO.hpp"
#include <vector>

namespace ImTimeline {
    void BeginTimeline();
    void EndTimeline();

    void RenderTrack(std::vector<LKeyframeCommon>& keyframes, int keyframeScale=5);
};
