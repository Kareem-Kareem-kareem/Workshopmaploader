#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"

class WorkshopMapLoader : public BakkesMod::Plugin::BakkesModPlugin {
public:
    void onLoad() override;
    void onUnload() override;
};
