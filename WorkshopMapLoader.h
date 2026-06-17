#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include <string>
#include <vector>
#include <filesystem>

using namespace BakkesMod::Plugin;

class WorkshopMapLoader : public BakkesModPlugin, public PluginWindow
{
private:
    std::vector<std::wstring> mapFiles_;
    std::string statusMsg_;
    bool isWindowOpen_;

public:
    void onLoad() override;
    void onUnload() override;

    void Render() override;
    std::string GetPluginName() override;
    void SetImGuiContext(uintptr_t ctx) override;

    void OnOpen() override;
    void OnClose() override;

    void RefreshMaps();
    void LoadMap(const std::wstring& mapPath);
};
