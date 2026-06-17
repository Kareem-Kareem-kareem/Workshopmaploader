#include "WorkshopMapLoader.h"
#include "imgui/imgui.h"
#include <fstream>
#include <algorithm>

BAKKESMOD_PLUGIN(WorkshopMapLoader, "Workshop Map Loader", "2.0", PLUGINTYPE_FREEPLAY)

#define BMLOG(msg) cvarManager->log(msg)

void WorkshopMapLoader::onLoad() {
    // Register the directory path variable to persist in config.cfg
    cvarManager->registerCvar("wml_directory", "C:\\Program Files (x86)\\Steam\\steamapps\\workshop\\content\\252950", "Default Workshop Maps Directory", true, false, 0, false, 0, true);
    cvarManager->registerCvar("wml_password", "", "LAN Server Password", true, false, 0, false, 0, true);

    // Load initial values from persistent variables
    std::string savedDir = cvarManager->getCvar("wml_directory").getStringValue();
    strncpy_s(mapDirBuf_, savedDir.c_str(), sizeof(mapDirBuf_) - 1);

    std::string savedPwd = cvarManager->getCvar("wml_password").getStringValue();
    strncpy_s(lanPasswordBuf_, savedPwd.c_str(), sizeof(lanPasswordBuf_) - 1);

    // Initial scan on startup
    ScanForMaps();

    // Console commands for binds
    cvarManager->registerNotifier("wml_scan", [this](std::vector<std::string> args) { ScanForMaps(); }, "Scan for maps", PERMISSION_ALL);
    cvarManager->registerNotifier("wml_enter", [this](std::vector<std::string> args) {
        if (selectedMapIdx_ >= 0 && selectedMapIdx_ < (int)maps_.size()) {
            EnterMap(maps_[selectedMapIdx_].path);
        }
    }, "Enter selected map", PERMISSION_ALL);
}

void WorkshopMapLoader::onUnload() {
}

void WorkshopMapLoader::SetImGuiContext(uintptr_t ctx) {
    ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

std::string WorkshopMapLoader::GetPluginName() {
    return "Workshop Map Loader";
}

void WorkshopMapLoader::ScanForMaps() {
    maps_.clear();
    selectedMapIdx_ = -1;
    std::string targetDir(mapDirBuf_);

    if (targetDir.empty() || !std::filesystem::exists(targetDir)) {
        statusMsg_ = "Error: Directory does not exist.";
        return;
    }

    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(targetDir)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                // Standard UDK extension forms
                if (ext == ".udk" || ext == ".upk") {
                    WorkshopMap m;
                    m.name = entry.path().stub().string();
                    m.path = entry.path().string();
                    maps_.push_back(m);
                }
            }
        }
        statusMsg_ = "Scan successful. Found " + std::to_string(maps_.size()) + " maps.";
        // Persist the directory configuration path
        cvarManager->getCvar("wml_directory").setValue(targetDir);
    }
    catch (const std::exception& e) {
        statusMsg_ = "Scan failed: " + std::string(e.what());
    }
}

void WorkshopMapLoader::EnterMap(const std::string& mapPath) {
    if (mapPath.empty()) return;
    
    statusMsg_ = "Loading solo map...";
    BMLOG("WML: Loading map via training data wrapper: " + mapPath);
    
    // Core approach for local custom map instances
    gameWrapper->GetGfxTrainingData().PlayFreeplayMap(mapPath);
}

void WorkshopMapLoader::CreateLanMatch() {
    if (selectedMapIdx_ < 0 || selectedMapIdx_ >= (int)maps_.size()) {
        statusMsg_ = "Error: Select a map first.";
        return;
    }

    std::string mapPath = maps_[selectedMapIdx_].path;
    std::string pwd(lanPasswordBuf_);
    
    // Update stored password
    cvarManager->getCvar("wml_password").setValue(pwd);

    statusMsg_ = "Hosting LAN match...";
    BMLOG("WML: Hosting network map instance: " + mapPath);

    // Build the execution parameters
    std::string cmd = "open \"" + mapPath + "\"?listen";
    if (!pwd.empty()) {
        cmd += "?password=" + pwd;
    }

    gameWrapper->ExecuteUnrealCommand(cmd);
}

void WorkshopMapLoader::Render() {
    ImGui::SetNextWindowSize(ImVec2(550, 450), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(GetPluginName().c_str(), &isWindowOpen_)) {
        ImGui::End();
        return;
    }

    // Paths Configuration Section
    ImGui::TextTextUnformatted("Map Directory Path:");
    if (ImGui::InputText("##dir", mapDirBuf_, sizeof(mapDirBuf_))) {
        // Keeps user update real-time
    }
    ImGui::SameLine();
    if (ImGui::Button("Scan / Refresh")) {
        ScanForMaps();
    }

    ImGui::Separator();

    // Map List Filtering
    ImGui::TextTextUnformatted("Search Filter:");
    ImGui::InputText("##search", searchBuf_, sizeof(searchBuf_));

    // Dynamic Scrolling Selection Box
    if (ImGui::BeginChild("MapListRegion", ImVec2(0, 200), true)) {
        std::string searchStr(searchBuf_);
        std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

        for (int i = 0; i < (int)maps_.size(); ++i) {
            std::string mapNameLower = maps_[i].name;
            std::transform(mapNameLower.begin(), mapNameLower.end(), mapNameLower.begin(), ::tolower);

            if (!searchStr.empty() && mapNameLower.find(searchStr) == std::string::npos) {
                continue;
            }

            const bool isSelected = (selectedMapIdx_ == i);
            if (ImGui::Selectable(maps_[i].name.c_str(), isSelected)) {
                selectedMapIdx_ = i;
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
            
            // Double-click shortcut action
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                selectedMapIdx_ = i;
                EnterMap(maps_[i].path);
            }
        }
        ImGui::EndChild();
    }

    ImGui::Separator();

    // Action Execution Controls
    const bool clearToRun = (selectedMapIdx_ >= 0 && selectedMapIdx_ < (int)maps_.size());

    // Faded Alpha state emulator for older ImGui versions
    if (!clearToRun) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);

    if (ImGui::Button("Enter Map", ImVec2(140, 40)) && clearToRun) {
        EnterMap(maps_[selectedMapIdx_].path);
    }
    
    if (!clearToRun) ImGui::PopStyleVar();

    ImGui::SameLine(0, 20);

    // LAN Management Interface
    ImGui::BeginGroup();
    ImGui::TextTextUnformatted("LAN Room Password:");
    ImGui::SetNextItemWidth(150);
    ImGui::InputText("##lan_pwd", lanPasswordBuf_, sizeof(lanPasswordBuf_));
    
    if (!clearToRun) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
    if (ImGui::Button("Create LAN Match") && clearToRun) {
        CreateLanMatch();
    }
    if (!clearToRun) ImGui::PopStyleVar();
    ImGui::EndGroup();

    ImGui::Separator();
    
    // Live Footer Info Feedback Tracker
    ImGui::TextTextUnformatted("Status:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), statusMsg_.c_str());

    ImGui::End();
}
