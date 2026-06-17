#pragma once

// Establish structural SDK links explicitly before type compilation begins
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"

#include <string>
#include <vector>
#include <filesystem>

struct WorkshopMap {
    std::string name;
    std::string path;
};

class WorkshopMapLoader : public BakkesModPlugin, public PluginWindow {
public:
    // Core BakkesMod Lifecycle Engine Hooks
    virtual void onLoad() override;
    virtual void onUnload() override;

    // ImGui / UI Rendering Subsystem Overrides
    virtual void Render() override;
    virtual std::string GetPluginName() override;
    virtual void SetImGuiContext(uintptr_t ctx) override;

    // Direct Overlay Canvas State Switches
    virtual void OnOpen() override;
    virtual void OnClose() override;

    // Core Plugin Functionality Routine Elements
    void ScanForMaps();
    void EnterMap(const std::string& mapPath);
    void CreateLanMatch();

private:
    // Memory Arrays and State Enums
    std::vector<WorkshopMap> maps_;
    int selectedMapIdx_ = -1;
    bool isWindowOpen_ = false;
    std::string statusMsg_ = "Ready";

    // Static Text Allocation Windows for Local Input Management
    char searchBuf_[256] = "";
    char mapDirBuf_[512] = "";
    char lanPasswordBuf_[128] = "";
};
