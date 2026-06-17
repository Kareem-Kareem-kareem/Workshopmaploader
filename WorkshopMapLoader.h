#pragma once

// Force include the specific absolute SDK interface components to resolve C2504
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginWindow.h"

#include <string>
#include <vector>
#include <filesystem>

struct WorkshopMap {
    std::string name;
    std::string path;
};

// Explicitly inherited using fully qualified namespace contexts if needed
class WorkshopMapLoader : public BakkesModPlugin, public PluginWindow {
public:
    // Core Lifecycle Hooks
    void onLoad() override;
    void onUnload() override;

    // Interface Subsystems
    void Render() override;
    std::string GetPluginName() override;
    void SetImGuiContext(uintptr_t ctx) override;

    // Window Visibility Overrides
    void OnOpen() override;
    void OnClose() override;

    // Operational Features
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
