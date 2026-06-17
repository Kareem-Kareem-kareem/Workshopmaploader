#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <string>

using namespace BakkesMod::Plugin;

class WorkshopMapLoader : public BakkesModPlugin {
public:
    void onLoad() override;
    void onUnload() override;

    void RenderCanvas(CanvasWrapper canvas);

private:
    int goals_ = 0;
    int saves_ = 0;
    int shots_ = 0;
    int assists_ = 0;
    float boostAmount_ = 0.0f;
};
