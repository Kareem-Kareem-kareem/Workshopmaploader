#include "WorkshopMapLoader.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <algorithm>
#include <cctype>

// ─── Plugin metadata ──────────────────────────────────────────────────────────
BAKKESMOD_PLUGIN(WorkshopMapLoader,
                 "Workshop Map Loader",
                 "1.1.0",
                 PLUGINTYPE_FREEPLAY)

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────────────
static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return (char)std::tolower(c); });
    return s;
}

// Safely copy into a fixed char buffer and return pointer
template<size_t N>
static void safeCopy(char (&dst)[N], const std::string& src) {
    strncpy_s(dst, src.c_str(), N - 1);
    dst[N - 1] = '\0';
}

// ─────────────────────────────────────────────────────────────────────────────
//  Lifecycle
// ─────────────────────────────────────────────────────────────────────────────
void WorkshopMapLoader::onLoad()
{
    // ── CVars ─────────────────────────────────────────────────────────────────
    enablePlugin_ = std::make_shared<bool>(true);
    workshopPath_ = std::make_shared<std::string>("");
    autoScan_     = std::make_shared<bool>(true);

    cvarManager->registerCvar("wml_enabled", "1",
        "Enable Workshop Map Loader", true, true, 0, true, 1)
        .bindTo(enablePlugin_);

    cvarManager->registerCvar("wml_workshop_path", "",
        "Root folder(s) to scan for .udk workshop maps. "
        "Leave blank to auto-detect.", true, false, 0, false, 0, true)
        .bindTo(workshopPath_);

    cvarManager->registerCvar("wml_auto_scan", "1",
        "Scan for maps automatically on plugin load", true, true, 0, true, 1)
        .bindTo(autoScan_);

    // ── Console notifiers ─────────────────────────────────────────────────────

    // wml_toggle  — open/close the overlay from the ~ console
    cvarManager->registerNotifier("wml_toggle", [this](std::vector<std::string>) {
        isWindowOpen_ = !isWindowOpen_;
        // BakkesMod needs to know the window state changed so it renders it
        if (isWindowOpen_) {
            gameWrapper->Execute([this](GameWrapper*) { cvarManager->executeCommand("togglemenu " + GetMenuName()); });
        } else {
            gameWrapper->Execute([this](GameWrapper*) { cvarManager->executeCommand("togglemenu " + GetMenuName()); });
        }
    }, "Toggle the Workshop Map Loader window", PERMISSION_ALL);

    // wml_open / wml_close — explicit versions
    cvarManager->registerNotifier("wml_open", [this](std::vector<std::string>) {
        gameWrapper->Execute([this](GameWrapper*) {
            cvarManager->executeCommand("togglemenu " + GetMenuName());
        });
    }, "Open the Workshop Map Loader window", PERMISSION_ALL);

    cvarManager->registerNotifier("wml_close", [this](std::vector<std::string>) {
        if (isWindowOpen_) {
            gameWrapper->Execute([this](GameWrapper*) {
                cvarManager->executeCommand("togglemenu " + GetMenuName());
            });
        }
    }, "Close the Workshop Map Loader window", PERMISSION_ALL);

    // wml_scan — rescan maps folder
    cvarManager->registerNotifier("wml_scan", [this](std::vector<std::string>) {
        ScanWorkshopMaps();
    }, "Rescan workshop folder for maps", PERMISSION_ALL);

    // wml_list — print all found maps to the BakkesMod console
    cvarManager->registerNotifier("wml_list", [this](std::vector<std::string>) {
        if (maps_.empty()) { LOG("No maps loaded. Run wml_scan first."); return; }
        for (int i = 0; i < (int)maps_.size(); ++i)
            LOG("[{}] {}", i, maps_[i].displayName);
    }, "List all found workshop maps in the console", PERMISSION_ALL);

    // wml_load <index|path>
    cvarManager->registerNotifier("wml_load", [this](std::vector<std::string> args) {
        if (args.size() < 2) {
            LOG("Usage:  wml_load <index>   or   wml_load \"C:/path/to/map.udk\"");
            LOG("Run wml_list to see available indices.");
            return;
        }
        try {
            int idx = std::stoi(args[1]);
            if (idx < 0 || idx >= (int)maps_.size()) {
                LOG("Index {} out of range (0–{}). Run wml_list.", idx, (int)maps_.size()-1);
                return;
            }
            LoadMap(maps_[idx]);
        } catch (...) {
            LoadMapByPath(args[1]);
        }
    }, "Load a workshop map by index (wml_list) or full file path", PERMISSION_ALL);

    // wml_return
    cvarManager->registerNotifier("wml_return", [this](std::vector<std::string>) {
        ReturnToMainMenu();
    }, "Return to the main menu from a workshop map", PERMISSION_ALL);

    // ── Auto-scan ─────────────────────────────────────────────────────────────
    if (*autoScan_) {
        ScanWorkshopMaps();
    }

    LOG("Workshop Map Loader v1.1 loaded. {} map(s) found.", maps_.size());
    LOG("Open with:  wml_open   or   togglemenu workshopmaploader");
    LOG("Load from console:  wml_list  then  wml_load <index>");
}

void WorkshopMapLoader::onUnload()
{
    LOG("Workshop Map Loader unloaded.");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Auto-detect workshop roots (Steam + Epic / manual)
// ─────────────────────────────────────────────────────────────────────────────
std::vector<fs::path> WorkshopMapLoader::FindWorkshopRoots()
{
    std::vector<fs::path> candidates;

    // 1. User-configured path (highest priority)
    if (!workshopPath_->empty())
        candidates.push_back(*workshopPath_);

    // 2. Standard Steam install locations
    candidates.push_back("C:/Program Files (x86)/Steam/steamapps/workshop/content/252950");
    candidates.push_back("C:/Program Files/Steam/steamapps/workshop/content/252950");
    candidates.push_back("D:/SteamLibrary/steamapps/workshop/content/252950");
    candidates.push_back("E:/SteamLibrary/steamapps/workshop/content/252950");

    // 3. BakkesMod's own custom maps folder — works for BOTH Steam AND Epic
    //    because BakkesMod copies workshop maps here when you subscribe via
    //    the in-game overlay.  This is the canonical path for Epic users.
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        fs::path bm(appdata);
        candidates.push_back(bm / "bakkesmod" / "bakkesmod" / "data" / "maps");
    }

    // 4. Common manual / community drop-folders
    const char* pub = "C:/Users/Public/Documents";
    candidates.push_back(fs::path(pub) / "rocketleague_workshop");

    // 5. RL's own CookedPCConsole (some older maps are placed here)
    //    Epic path:
    candidates.push_back(
        "C:/Program Files/Epic Games/rocketleague/TAGame/CookedPCConsole");
    //    Steam path:
    candidates.push_back(
        "C:/Program Files (x86)/Steam/steamapps/common/rocketleague/TAGame/CookedPCConsole");

    // Deduplicate and keep only existing directories
    std::vector<fs::path> result;
    for (auto& p : candidates) {
        if (fs::exists(p) && fs::is_directory(p)) {
            // Simple dedup
            bool already = false;
            for (auto& r : result) {
                std::error_code ec;
                if (fs::equivalent(r, p, ec)) { already = true; break; }
            }
            if (!already) result.push_back(p);
        }
    }
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Map scanning
// ─────────────────────────────────────────────────────────────────────────────
void WorkshopMapLoader::ScanWorkshopMaps()
{
    maps_.clear();
    statusMsg_ = "Scanning…";

    auto roots = FindWorkshopRoots();
    if (roots.empty()) {
        statusMsg_ = "No workshop folders found. Set path manually above.";
        LOG("{}", statusMsg_);
        return;
    }

    for (auto& root : roots) {
        try {
            for (auto& entry : fs::recursive_directory_iterator(
                     root, fs::directory_options::skip_permission_denied))
            {
                auto ext = toLower(entry.path().extension().string());
                if (ext != ".udk" && ext != ".upk") continue;

                WorkshopMap wm;
                wm.filePath = entry.path();
                wm.name     = entry.path().stem().string();

                // Build a readable display name:
                // "<workshop_item_id_or_folder> / <stem>"
                auto parent = entry.path().parent_path().filename().string();
                wm.displayName = parent + " / " + wm.name;

                maps_.push_back(std::move(wm));
            }
        } catch (const std::exception& e) {
            LOG("Scan warning for {}: {}", root.string(), e.what());
        }
    }

    statusMsg_ = "Found " + std::to_string(maps_.size()) + " map(s) in "
               + std::to_string(roots.size()) + " folder(s).";
    LOG("{}", statusMsg_);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Loading
// ─────────────────────────────────────────────────────────────────────────────
void WorkshopMapLoader::LoadMap(const WorkshopMap& map)
{
    LoadMapByPath(map.filePath.string());
}

void WorkshopMapLoader::LoadMapByPath(const std::string& path)
{
    if (!*enablePlugin_) { LOG("wml: plugin is disabled."); return; }

    if (!fs::exists(path)) {
        statusMsg_ = "File not found: " + path;
        LOG("{}", statusMsg_);
        return;
    }

    statusMsg_ = "Loading: " + fs::path(path).filename().string();
    LOG("{}", statusMsg_);

    // gameWrapper->LoadMap() is BakkesMod's hook into RL's map loader.
    // It works identically for Steam and Epic builds.
    gameWrapper->Execute([this, path](GameWrapper*) {
        gameWrapper->LoadMap(path);
    });

    statusMsg_ = "Loaded: " + fs::path(path).filename().string();
}

void WorkshopMapLoader::ReturnToMainMenu()
{
    gameWrapper->Execute([this](GameWrapper*) {
        gameWrapper->ExecuteUnrealCommand("disconnect");
    });
    statusMsg_ = "Returned to main menu.";
    LOG("{}", statusMsg_);
}

// ─────────────────────────────────────────────────────────────────────────────
//  ImGui window — rendered by BakkesMod via PluginWindow
// ─────────────────────────────────────────────────────────────────────────────
void WorkshopMapLoader::SetImGuiContext(uintptr_t ctx)
{
    ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

std::string WorkshopMapLoader::GetMenuName()  { return "workshopmaploader"; }
std::string WorkshopMapLoader::GetMenuTitle() { return "Workshop Map Loader"; }

void WorkshopMapLoader::Render()
{
    if (!isWindowOpen_) return;

    ImGui::SetNextWindowSize(ImVec2(640, 540), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(100, 80),   ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (!ImGui::Begin("Workshop Map Loader  [wml_open / wml_close]",
                      &isWindowOpen_, flags))
    {
        ImGui::End();
        return;
    }

    // ── Info bar (console hint) ───────────────────────────────────────────────
    ImGui::TextColored(ImVec4(0.5f,0.9f,0.5f,1.f),
        "Console commands: wml_open  wml_list  wml_load <n>  wml_return");
    ImGui::Separator();
    ImGui::Spacing();

    // ── Workshop path ─────────────────────────────────────────────────────────
    ImGui::Text("Maps folder (blank = auto-detect Steam + Epic paths):");
    // Keep buffer in sync with cvar on first render
    if (pathBuf_[0] == '\0' && !workshopPath_->empty())
        safeCopy(pathBuf_, *workshopPath_);

    ImGui::SetNextItemWidth(-80);
    bool pathEdited = ImGui::InputText("##wspath", pathBuf_, sizeof(pathBuf_),
                                       ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    bool scanClicked = ImGui::Button("Scan##btn", ImVec2(70, 0));

    if (pathEdited || scanClicked) {
        *workshopPath_ = pathBuf_;
        cvarManager->getCvar("wml_workshop_path").setValue(*workshopPath_);
        ScanWorkshopMaps();
    }

    ImGui::Spacing();

    // ── Direct path load ──────────────────────────────────────────────────────
    ImGui::Text("Or load a .udk file directly:");
    ImGui::SetNextItemWidth(-130);
    ImGui::InputText("##custompath", customPathBuf_, sizeof(customPathBuf_));
    ImGui::SameLine();
    if (ImGui::Button("Load File", ImVec2(120, 0))) {
        if (customPathBuf_[0] != '\0')
            LoadMapByPath(std::string(customPathBuf_));
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ── Search ────────────────────────────────────────────────────────────────
    ImGui::Text("Search:"); ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##search", searchBuf_, sizeof(searchBuf_));

    // ── Map list ──────────────────────────────────────────────────────────────
    float listH = ImGui::GetContentRegionAvail().y - 56.0f;
    ImGui::BeginChild("##maplist", ImVec2(0, listH), true);

    if (maps_.empty()) {
        ImGui::TextDisabled("No maps found.");
        ImGui::TextDisabled("Click Scan, or for Epic Games users:");
        ImGui::TextDisabled("  - Download maps via BakkesMod Workshop browser");
        ImGui::TextDisabled("    (they go to %%APPDATA%%\\bakkesmod\\bakkesmod\\data\\maps)");
        ImGui::TextDisabled("  - Or paste a path above and click Load File.");
    }

    std::string filter = toLower(std::string(searchBuf_));
    for (int i = 0; i < (int)maps_.size(); ++i) {
        const auto& m = maps_[i];
        if (!filter.empty() && toLower(m.displayName).find(filter) == std::string::npos)
            continue;

        bool selected = (selectedMapIndex_ == i);
        char label[600];
        snprintf(label, sizeof(label), "[%d]  %s", i, m.displayName.c_str());

        if (ImGui::Selectable(label, selected, ImGuiSelectableFlags_AllowDoubleClick)) {
            selectedMapIndex_ = i;
            if (ImGui::IsMouseDoubleClicked(0))
                LoadMap(m);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", m.filePath.string().c_str());
            ImGui::Text("Double-click to load instantly");
            ImGui::EndTooltip();
        }
    }
    ImGui::EndChild();

    // ── Buttons ───────────────────────────────────────────────────────────────
    ImGui::Spacing();
    bool canLoad = (selectedMapIndex_ >= 0 && selectedMapIndex_ < (int)maps_.size());
    if (!canLoad) ImGui::BeginDisabled();
    if (ImGui::Button("Load Selected", ImVec2(150, 0)))
        LoadMap(maps_[selectedMapIndex_]);
    if (!canLoad) ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button("Return to Menu", ImVec2(150, 0)))
        ReturnToMainMenu();

    ImGui::SameLine();
    ImGui::TextDisabled("  %s", statusMsg_.c_str());

    ImGui::End();
}
