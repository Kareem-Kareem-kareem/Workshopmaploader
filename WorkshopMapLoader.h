#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <string>
#include <memory>

using namespace BakkesMod::Plugin;

class WorkshopMapLoader : public BakkesModPlugin {
public:
    void onLoad() override;
    void onUnload() override;

    void OnPhysicsTick(std::string eventName);
    void OnBallHit(std::string eventName);

private:
    bool isAutoAimActive_ = false;
};
