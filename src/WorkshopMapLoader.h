#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
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
    // BakkesModPlugin
    void onLoad()   override;
    void onUnload() override;

    // PluginWindow — used by F2 menu AND our own overlay toggle
    void        Render()           override;
    std::string GetMenuName()      override;
    std::string GetMenuTitle()     override;
    void        SetImGuiContext(uintptr_t ctx) override;
    bool        ShouldBlockInput() override { return false; }
    bool        IsActiveOverlay()  override { return true;  }
    void        OnOpen()           override { isWindowOpen_ = true;  }
    void        OnClose()          override { isWindowOpen_ = false; }

private:
    void ScanWorkshopMaps();
    void LoadMap(const WorkshopMap& map);
    void LoadMapByPath(const std::string& path);
    void ReturnToMainMenu();

    // Finds all plausible workshop roots on this machine (Steam + Epic)
    std::vector<fs::path> FindWorkshopRoots();

    // CVars
    std::shared_ptr<bool>        enablePlugin_;
    std::shared_ptr<std::string> workshopPath_;
    std::shared_ptr<bool>        autoScan_;

    // State
    std::vector<WorkshopMap> maps_;
    int         selectedMapIndex_ = -1;
    bool        isWindowOpen_     = false;
    char        searchBuf_[256]   = {};
    char        pathBuf_[512]     = {};   // kept in sync with workshopPath_
    char        customPathBuf_[512] = {};
    std::string statusMsg_;

    // Epic Games default workshop path (same content, different Steam install dir)
    // BakkesMod injects into RL regardless of store; maps are still UDK files.
    static constexpr const char* DEFAULT_STEAM_PATH =
        "C:/Program Files (x86)/Steam/steamapps/workshop/content/252950";
    // Epic users share the same workshop files IF they also have Steam RL,
    // but many only have Epic — they store maps manually.  We expose a
    // configurable path and auto-detect common locations.
    static constexpr const char* DEFAULT_EPIC_MAPS_PATH =
        "C:/Users/Public/Documents/rocketleague_workshop";
};
