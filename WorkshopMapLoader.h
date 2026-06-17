#pragma once
#pragma comment(lib, "pluginsdk.lib")

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
    // Core BakkesMod Lifecycle Hooks
    virtual void onLoad() override;
    virtual void onUnload() override;

    // ImGui / Overlay Subsystem Interfaces
    virtual void Render() override;
    virtual std::string GetPluginName() override;
    virtual void SetImGuiContext(uintptr_t ctx) override;

    // Window State Management Hooks
    virtual void OnOpen() override;
    virtual void OnClose() override;

    // System Features and Event Handlers
    void ScanForMaps();
    void EnterMap(const std::string& mapPath);
    void CreateLanMatch();

private:
    // Plugin State Variables
    std::vector<WorkshopMap> maps_;
    int selectedMapIdx_ = -1;
    bool isWindowOpen_ = false;
    std::string statusMsg_ = "Ready";

    // Text Input Buffers for UI Controls
    char searchBuf_[256] = "";
    char mapDirBuf_[512] = "";
    char lanPasswordBuf_[128] = "";
};
