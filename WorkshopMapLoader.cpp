#include "WorkshopMapLoader.h"
#include <algorithm>
#include <cctype>

BAKKESMOD_PLUGIN(WorkshopMapLoader, "Workshop Map Loader", "1.1.0", PLUGINTYPE_FREEPLAY)

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return (char)std::tolower(c); });
    return s;
}

template<size_t N>
static void safeCopy(char (&dst)[N], const std::string& src) {
    strncpy_s(dst, src.c_str(), N - 1);
    dst[N - 1] = '\0';
}

// Shorthand: log to BakkesMod console
#define BMLOG(msg) cvarManager->log(msg)

// ── Lifecycle ─────────────────────────────────────────────────────────────────

void WorkshopMapLoader::onLoad()
{
    enablePlugin_ = std::make_shared<bool>(true);
    workshopPath_ = std::make_shared<std::string>("");
    autoScan_     = std::make_shared<bool>(true);

    cvarManager->registerCvar("wml_enabled", "1",
        "Enable Workshop Map Loader", true, true, 0, true, 1)
        .bindTo(enablePlugin_);

    cvarManager->registerCvar("wml_workshop_path", "",
        "Root folder to scan for .udk maps (blank = auto-detect)", true, false, 0, false, 0, true)
        .bindTo(workshopPath_);

    cvarManager->registerCvar("wml_auto_scan", "1",
        "Scan for maps on plugin load", true, true, 0, true, 1)
        .bindTo(autoScan_);

    // ── Console commands ──────────────────────────────────────────────────────

    // Open the window from the ~ console
    cvarManager->registerNotifier("wml_open", [this](std::vector<std::string>) {
        gameWrapper->Execute([this](GameWrapper*) {
            cvarManager->executeCommand("togglemenu " + GetMenuName());
        });
    }, "Open Workshop Map Loader window", PERMISSION_ALL);

    cvarManager->registerNotifier("wml_close", [this](std::vector<std::string>) {
        if (isWindowOpen_) {
            gameWrapper->Execute([this](GameWrapper*) {
                cvarManager->executeCommand("togglemenu " + GetMenuName());
            });
        }
    }, "Close Workshop Map Loader window", PERMISSION_ALL);

    cvarManager->registerNotifier("wml_scan", [this](std::vector<std::string>) {
        ScanWorkshopMaps();
    }, "Rescan workshop folders for maps", PERMISSION_ALL);

    cvarManager->registerNotifier("wml_list", [this](std::vector<std::string>) {
        if (maps_.empty()) {
            BMLOG("No maps found. Run wml_scan first.");
            return;
        }
        for (int i = 0; i < (int)maps_.size(); ++i)
            BMLOG("[" + std::to_string(i) + "] " + maps_[i].displayName);
    }, "List all found workshop maps", PERMISSION_ALL);

    // wml_load <index|path>
    cvarManager->registerNotifier("wml_load", [this](std::vector<std::string> args) {
        if (args.size() < 2) {
            BMLOG("Usage: wml_load <index>  or  wml_load \"C:/path/to/map.udk\"");
            BMLOG("Run wml_list to see map indices.");
            return;
        }
        try {
            int idx = std::stoi(args[1]);
            if (idx < 0 || idx >= (int)maps_.size()) {
                BMLOG("Index " + std::to_string(idx) + " out of range (0-"
                    + std::to_string((int)maps_.size()-1) + "). Run wml_list.");
                return;
            }
            LoadMap(maps_[idx]);
        } catch (...) {
            LoadMapByPath(args[1]);
        }
    }, "Load workshop map by index or path", PERMISSION_ALL);

    cvarManager->registerNotifier("wml_return", [this](std::vector<std::string>) {
        ReturnToMainMenu();
    }, "Return to main menu", PERMISSION_ALL);

    if (*autoScan_) ScanWorkshopMaps();

    BMLOG("Workshop Map Loader loaded. " + std::to_string(maps_.size()) + " map(s) found.");
    BMLOG("Open the map browser: wml_open");
}

void WorkshopMapLoader::onUnload()
{
    BMLOG("Workshop Map Loader unloaded.");
}

// ── Map path detection ────────────────────────────────────────────────────────

std::vector<fs::path> WorkshopMapLoader::FindWorkshopRoots()
{
    std::vector<fs::path> candidates;

    // 1. User-configured path (highest priority)
    if (!workshopPath_->empty())
        candidates.push_back(*workshopPath_);

    // 2. BakkesMod data folder — works for BOTH Steam AND Epic
    //    BakkesMod downloads workshop maps here when you use F2 > Workshop
    fs::path bmDataMaps = gameWrapper->GetDataFolder() / "maps";
    candidates.push_back(bmDataMaps);

    // 3. Steam workshop paths (C/D/E drives)
    for (const char* drive : {"C", "D", "E", "F"}) {
        candidates.push_back(
            std::string(drive) + ":/Program Files (x86)/Steam/steamapps/workshop/content/252950");
        candidates.push_back(
            std::string(drive) + ":/SteamLibrary/steamapps/workshop/content/252950");
    }

    // 4. Epic Games RL install (CookedPCConsole holds some maps)
    candidates.push_back("C:/Program Files/Epic Games/rocketleague/TAGame/CookedPCConsole");

    // 5. Public Documents fallback for manual drops
    candidates.push_back("C:/Users/Public/Documents/rocketleague_workshop");

    // Keep only directories that exist, deduplicated
    std::vector<fs::path> result;
    for (auto& p : candidates) {
        std::error_code ec;
        if (!fs::is_directory(p, ec)) continue;
        bool dup = false;
        for (auto& r : result) {
            std::error_code ec2;
            if (fs::equivalent(r, p, ec2) && !ec2) { dup = true; break; }
        }
        if (!dup) result.push_back(p);
    }
    return result;
}

// ── Scanning ──────────────────────────────────────────────────────────────────

void WorkshopMapLoader::ScanWorkshopMaps()
{
    maps_.clear();
    statusMsg_ = "Scanning...";

    auto roots = FindWorkshopRoots();
    if (roots.empty()) {
        statusMsg_ = "No workshop folders found. Set a path manually.";
        BMLOG(statusMsg_);
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
                wm.filePath    = entry.path();
                wm.name        = entry.path().stem().string();
                wm.displayName = entry.path().parent_path().filename().string()
                               + " / " + wm.name;
                maps_.push_back(std::move(wm));
            }
        } catch (const std::exception& e) {
            BMLOG(std::string("Scan warning: ") + e.what());
        }
    }

    statusMsg_ = "Found " + std::to_string(maps_.size()) + " map(s).";
    BMLOG(statusMsg_);
}

// ── Loading ───────────────────────────────────────────────────────────────────

void WorkshopMapLoader::LoadMap(const WorkshopMap& map)
{
    LoadMapByPath(map.filePath.string());
}

void WorkshopMapLoader::LoadMapByPath(const std::string& path)
{
    if (!*enablePlugin_) { BMLOG("Workshop Map Loader is disabled."); return; }

    std::error_code ec;
    if (!fs::exists(path, ec)) {
        statusMsg_ = "File not found: " + path;
        BMLOG(statusMsg_);
        return;
    }

    statusMsg_ = "Loading: " + fs::path(path).filename().string();
    BMLOG(statusMsg_);

    // "open <path>" is the correct Unreal console command to load a map.
    // Works identically on Steam and Epic builds.
    std::string cmd = "open \"" + path + "\"";
    gameWrapper->Execute([this, cmd](GameWrapper*) {
        gameWrapper->ExecuteUnrealCommand(cmd);
    });

    statusMsg_ = "Loaded: " + fs::path(path).filename().string();
}

void WorkshopMapLoader::ReturnToMainMenu()
{
    gameWrapper->Execute([this](GameWrapper*) {
        gameWrapper->ExecuteUnrealCommand("disconnect");
    });
    statusMsg_ = "Returned to main menu.";
    BMLOG(statusMsg_);
}

// ── ImGui window ──────────────────────────────────────────────────────────────

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

    if (!ImGui::Begin("Workshop Map Loader", &isWindowOpen_, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    // Console hint bar
    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.f),
        "~ console: wml_open  wml_list  wml_load <n>  wml_return");
    ImGui::Separator();
    ImGui::Spacing();

    // ── Workshop path ─────────────────────────────────────────────────────────
    ImGui::Text("Maps folder (blank = auto-detect):");
    if (pathBuf_[0] == '\0' && !workshopPath_->empty())
        safeCopy(pathBuf_, *workshopPath_);

    ImGui::SetNextItemWidth(-80.0f);
    bool pathEnter = ImGui::InputText("##wspath", pathBuf_, sizeof(pathBuf_),
                                      ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    bool scanBtn = ImGui::Button("Scan", ImVec2(70, 0));

    if (pathEnter || scanBtn) {
        *workshopPath_ = pathBuf_;
        cvarManager->getCvar("wml_workshop_path").setValue(*workshopPath_);
        ScanWorkshopMaps();
    }

    ImGui::Spacing();

    // ── Direct path ───────────────────────────────────────────────────────────
    ImGui::Text("Load .udk file directly:");
    ImGui::SetNextItemWidth(-130.0f);
    ImGui::InputText("##custom", customPathBuf_, sizeof(customPathBuf_));
    ImGui::SameLine();
    if (ImGui::Button("Load File", ImVec2(120, 0)) && customPathBuf_[0] != '\0')
        LoadMapByPath(std::string(customPathBuf_));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ── Search ────────────────────────────────────────────────────────────────
    ImGui::Text("Search:"); ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputText("##search", searchBuf_, sizeof(searchBuf_));

    // ── Map list ──────────────────────────────────────────────────────────────
    float listH = ImGui::GetContentRegionAvail().y - 56.0f;
    ImGui::BeginChild("##maplist", ImVec2(0, listH), true);

    if (maps_.empty()) {
        ImGui::TextDisabled("No maps found.");
        ImGui::TextDisabled("Epic Games users: use F2 > Workshop to download maps,");
        ImGui::TextDisabled("or paste a .udk path above and click Load File.");
    }

    std::string filter = toLower(std::string(searchBuf_));
    for (int i = 0; i < (int)maps_.size(); ++i) {
        const auto& m = maps_[i];
        if (!filter.empty() && toLower(m.displayName).find(filter) == std::string::npos)
            continue;

        bool selected = (selectedMapIndex_ == i);
        char label[640];
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
    if (!canLoad) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
    if (ImGui::Button("Load Selected", ImVec2(150, 0)) && canLoad)
        LoadMap(maps_[selectedMapIndex_]);
    if (!canLoad) ImGui::PopStyleVar();

    ImGui::SameLine();
    if (ImGui::Button("Return to Menu", ImVec2(150, 0)))
        ReturnToMainMenu();

    ImGui::SameLine();
    ImGui::TextDisabled("  %s", statusMsg_.c_str());

    ImGui::End();
}
