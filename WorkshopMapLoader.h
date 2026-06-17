#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "imgui.h"
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

struct WorkshopMap {
    std::string name;
    std::string displayName;
    fs::path    filePath;
};

class WorkshopMapLoader : public BakkesMod::Plugin::BakkesModPlugin,
                          public BakkesMod::Plugin::PluginWindow
{
public:
    void onLoad()   override;
    void onUnload() override;

    void        Render()                       override;
    std::string GetMenuName()                  override;
    std::string GetMenuTitle()                 override;
    void        SetImGuiContext(uintptr_t ctx) override;
    bool        ShouldBlockInput()             override { return false; }
    bool        IsActiveOverlay()              override { return true;  }
    void        OnOpen()                       override { isWindowOpen_ = true;  }
    void        OnClose()                      override { isWindowOpen_ = false; }

private:
    void ScanWorkshopMaps();
    void LoadMap(const WorkshopMap& map);
    void LoadMapByPath(const std::string& path);
    void HostLAN(const std::string& path);
    void ReturnToMainMenu();

    std::vector<fs::path> FindWorkshopRoots();

    // CVars
    std::shared_ptr<bool>        enablePlugin_;
    std::shared_ptr<std::string> workshopPath_;
    std::shared_ptr<bool>        autoScan_;

    // State
    std::vector<WorkshopMap> maps_;
    int  selectedMapIndex_ = -1;
    bool isWindowOpen_     = false;
    char searchBuf_[256]   = {};
    char pathBuf_[512]     = {};
    char customPathBuf_[512] = {};
    char lanPortBuf_[8]    = {"7777"};
    std::string statusMsg_;
};
