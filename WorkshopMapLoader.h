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
    virtual void onLoad() override;
    virtual void onUnload() override;

    // PluginWindow Implementation
    virtual void Render() override;
    virtual std::string GetPluginName() override;
    virtual void SetImGuiContext(uintptr_t ctx) override;

    // Core Logic
    void ScanForMaps();
    void EnterMap(const std::string& mapPath);
    void CreateLanMatch();

private:
    std::vector<WorkshopMap> maps_;
    int selectedMapIdx_ = -1;
    char searchBuf_[256] = "";
    char mapDirBuf_[512] = "";
    char lanPasswordBuf_[128] = "";
    std::string statusMsg_ = "Ready";
};
