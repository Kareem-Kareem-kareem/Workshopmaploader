#pragma once

#define WIN32_LEAN_AND_MEAN
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/wrappers/GameObject/CVarManagerWrapper.h"
#include <string>
#include <vector>

class WorkshopMapLoader : public BakkesMod::Plugin::BakkesModPlugin {
public:
    void onLoad() override;
    void onUnload() override;
};
