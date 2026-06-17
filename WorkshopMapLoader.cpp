#include "WorkshopMapLoader.h"
#include "imgui/imgui.h"
#include <fstream>

using namespace BakkesMod::Plugin;

BAKKESMOD_PLUGIN(WorkshopMapLoader, "Workshop Map Loader", "2.0", PLUGINTYPE_FREEPLAY)

void WorkshopMapLoader::onLoad()
{
    statusMsg_ = "Ready";
    isWindowOpen_ = false;

    cvarManager->registerNotifier("workshop_refresh", [this](std::vector<std::string> args) {
        RefreshMaps();
    }, "Refresh custom workshop maps list", DESCRIPTION_NONE);

    cvarManager->registerNotifier("workshop_load", [this](std::vector<std::string> args) {
        if (args.size() < 2) {
            cvarManager->log("Usage: workshop_load <map_name>");
            return;
        }
        std::string mapName = args[1];
        std::wstring wMapName(mapName.begin(), mapName.end());
        LoadMap(wMapName);
    }, "Load a specific workshop map by name", DESCRIPTION_NONE);

    RefreshMaps();
}

void WorkshopMapLoader::onUnload()
{
}

void WorkshopMapLoader::Render()
{
    if (!isWindowOpen_) return;

    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Workshop Map Loader", &isWindowOpen_)) {
        ImGui::End();
        return;
    }

    ImGui::Text("Status: %s", statusMsg_.c_str());

    if (ImGui::Button("Refresh Maps")) {
        RefreshMaps();
    }

    ImGui::Separator();

    ImGui::BeginChild("MapList");
    for (const auto& mapPath : mapFiles_) {
        std::wstring wFilename = std::filesystem::path(mapPath).stem().wstring();
        std::string filename(wFilename.begin(), wFilename.end());

        if (ImGui::Button(filename.c_str(), ImVec2(-1, 0))) {
            LoadMap(mapPath);
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

std::string WorkshopMapLoader::GetPluginName()
{
    return "Workshop Map Loader";
}

void WorkshopMapLoader::SetImGuiContext(uintptr_t ctx)
{
    ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

void WorkshopMapLoader::OnOpen()
{
    isWindowOpen_ = true;
}

void WorkshopMapLoader::OnClose()
{
    isWindowOpen_ = false;
}

void WorkshopMapLoader::RefreshMaps()
{
    mapFiles_.clear();
    std::filesystem::path bpath = gameWrapper->GetBakkesModPath();
    std::filesystem::path workshopFolder = bpath / "data" / "WorkshopMapLoader";

    if (!std::filesystem::exists(workshopFolder)) {
        std::filesystem::create_directories(workshopFolder);
        statusMsg_ = "Folder created. Place .udk/.upk files in: " + workshopFolder.string();
        return;
    }

    try {
        for (const auto& entry : std::filesystem::directory_iterator(workshopFolder)) {
            if (entry.is_regular_file()) {
                auto ext = entry.path().extension();
                if (ext == ".udk" || ext == ".upk") {
                    mapFiles_.push_back(entry.path().wstring());
                }
            }
        }
        statusMsg_ = "Found " + std::to_string(mapFiles_.size()) + " maps.";
    }
    catch (const std::exception& e) {
        statusMsg_ = "Error scanning folder: " + std::string(e.what());
    }
}

void WorkshopMapLoader::LoadMap(const std::wstring& mapPath)
{
    std::filesystem::path p(mapPath);
    std::string mapName = p.stem().string();

    if (!gameWrapper->IsInFreeplay()) {
        cvarManager->executeCommand("load_freeplay " + mapName);
        statusMsg_ = "Loading map: " + mapName;
    } else {
        cvarManager->executeCommand("switch_freeplay " + mapName);
        statusMsg_ = "Switching map: " + mapName;
    }
}
