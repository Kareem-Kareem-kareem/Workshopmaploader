#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <string>
#include <vector>

using namespace BakkesMod::Plugin;

class WorkshopMapLoader : public BakkesModPlugin {
public:
    void onLoad() override;
    void onUnload() override;
};
