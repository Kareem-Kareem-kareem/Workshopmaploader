#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"

#include <string>
#include <vector>

struct WorkshopMap {
    std::string name;
    std::string path;
};

class WorkshopMapLoader : public BakkesModPlugin, public PluginWindow {
public:
    // Core Engine Hooks
    void onLoad() override;
    void onUnload() override;

    // UI Panel Controls
    void Render() override;
    std::string GetPluginName() override;
    void SetImGuiContext(uintptr_t ctx) override;

    // Menu States
    void OnOpen() override;
    void OnClose() override;

    // Workspace Systems
    void ScanForMaps();
    void EnterMap(const std::string& mapPath);
    void CreateLanMatch();

private:
    std::vector<WorkshopMap> maps_;
    int selectedMapIdx_ = -1;
    bool isWindowOpen_ = false;
    std::string statusMsg_ = "Ready";

    char searchBuf_[256] = "";
    char mapDirBuf_[512] = "";
    char lanPasswordBuf_[128] = "";
};
