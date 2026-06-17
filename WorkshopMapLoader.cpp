#include "WorkshopMapLoader.h"

// Ensure the ImGui and core system definitions load explicitly after the header
#include "imgui/imgui.h"
#include <fstream>
#include <algorithm>
#include <exception>

// The registration macro connects 'cvarManager' and 'gameWrapper' behind the scenes.
BAKKESMOD_PLUGIN(WorkshopMapLoader, "Workshop Map Loader", "2.0", PLUGINTYPE_FREEPLAY)

#define BMLOG(msg) if (cvarManager) { cvarManager->log(msg); }

void WorkshopMapLoader::onLoad() {
    if (!cvarManager) return;

    cvarManager->registerCvar("wml_directory", "C:\\Program Files (x86)\\Steam\\steamapps\\workshop\\content\\252950", "Default Workshop Maps Directory", true, false, 0, false, 0, true);
    cvarManager->registerCvar("wml_password", "", "LAN Server Password", true, false, 0, false, 0, true);

    std::string savedDir = cvarManager->getCvar("wml_directory").getStringValue();
    if (!savedDir.empty()) {
        strncpy_s(mapDirBuf_, savedDir.c_str(), sizeof(mapDirBuf_) - 1);
        mapDirBuf_[sizeof(mapDirBuf_) - 1] = '\0';
    }

    std::string savedPwd = cvarManager->getCvar("wml_password").getStringValue();
    if (!savedPwd.empty()) {
        strncpy_s(lanPasswordBuf_, savedPwd.c_str(), sizeof(lanPasswordBuf_) - 1);
        lanPasswordBuf_[sizeof(lanPasswordBuf_) - 1] = '\0';
    }

    ScanForMaps();

    cvarManager->registerNotifier("wml_scan", [this](std::vector<std::string> args) {
        ScanForMaps();
    }, "Scan the target directory for custom maps instantly", PERMISSION_ALL);

    cvarManager->registerNotifier("wml_enter", [this](std::vector<std::string> args) {
        if (selectedMapIdx_ >= 0 && selectedMapIdx_ < (int)maps_.size()) {
            EnterMap(maps_[selectedMapIdx_].path);
        } else {
            statusMsg = "Error: Cannot execute console load. No map selected.";
            BMLOG("WML Error: Console command wml_enter failed due to no active map selection.");
        }
    }, "Load the currently selected map instantly", PERMISSION_ALL);

    cvarManager->registerNotifier("wml_toggle", [this](std::vector<std::string> args) {
        isWindowOpen_ = !isWindowOpen_;
        if (isWindowOpen_) {
            OnOpen();
        } else {
            OnClose();
        }
        BMLOG("WML: Interface visibility toggled via console command.");
    }, "Toggle the map loader UI panel overlay window directly via console", PERMISSION_ALL);
}

void WorkshopMapLoader::onUnload() {
    BMLOG("WML: Plugin successfully unloaded cleanly.");
}

void WorkshopMapLoader::SetImGuiContext(uintptr_t ctx) {
    ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

std::string WorkshopMapLoader::GetPluginName() {
    return "Workshop Map Loader";
}

void WorkshopMapLoader::OnOpen() {
    isWindowOpen_ = true;
    BMLOG("WML: Main menu interface window opened.");
}

void WorkshopMapLoader::OnClose() {
    isWindowOpen_ = false;
    BMLOG("WML: Main menu interface window closed.");
}

void WorkshopMapLoader::ScanForMaps() {
    maps_.clear();
    selectedMapIdx_ = -1;
    std::string targetDir(mapDirBuf_);

    targetDir.erase(std::remove(targetDir.begin(), targetDir.end(), '\"'), targetDir.end());
    while(!targetDir.empty() && std::isspace(targetDir.back())) {
        targetDir.pop_back();
    }

    if (targetDir.empty()) {
        statusMsg_ = "Error: Target map directory path string is empty.";
        return;
    }

    if (!std::filesystem::exists(targetDir)) {
        statusMsg_ = "Error: Configured directory pathway path does not exist.";
        return;
    }

    try {
        if (std::filesystem::is_directory(targetDir)) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(targetDir, std::filesystem::directory_options::skip_permission_denied)) {
                try {
                    if (entry.is_regular_file()) {
                        std::string ext = entry.path().extension().string();
                        if (ext == ".udk" || ext == ".upk") {
                            WorkshopMap m;
                            m.name = entry.path().stem().string();
                            m.path = entry.path().string();
                            maps_.push_back(m);
                        }
                    }
                }
                catch (const std::exception& innerEx) {
                    BMLOG("WML Warning: Skipping unreadable filesystem entry element item loop: " + std::string(innerEx.what()));
                    continue;
                }
            }
            
            statusMsg_ = "Scan successful. Found " + std::to_string(maps_.size()) + " maps.";
            BMLOG("WML Success: Directory parsed. Active array tracking updated with " + std::to_string(maps_.size()) + " maps.");
            
            if (cvarManager) {
                cvarManager->getCvar("wml_directory").setValue(targetDir);
            }
        } else {
            statusMsg_ = "Error: Selected target path is a file, not a directory.";
        }
    }
    catch (const std::exception& e) {
        statusMsg_ = "Scan failed: " + std::string(e.what());
        BMLOG("WML Critical Error: Exception caught during file indexing hierarchy traversal: " + std::string(e.what()));
    }
}

void WorkshopMapLoader::EnterMap(const std::string& mapPath) {
    if (mapPath.empty()) {
        statusMsg_ = "Error: Chosen map path context tracking is invalid.";
        return;
    }
    
    statusMsg_ = "Loading solo map...";
    BMLOG("WML Execution: Initializing load flow protocol wrapper route tracking for path: " + mapPath);
    
    if (gameWrapper) {
        gameWrapper->GetGfxTrainingData().PlayFreeplayMap(mapPath);
    }
}

void WorkshopMapLoader::CreateLanMatch() {
    if (selectedMapIdx_ < 0 || selectedMapIdx_ >= (int)maps_.size()) {
        statusMsg_ = "Error: Select a map from the listing interface hierarchy first.";
        return;
    }

    std::string mapPath = maps_[selectedMapIdx_].path;
    std::string pwd(lanPasswordBuf_);
    
    if (cvarManager) {
        cvarManager->getCvar("wml_password").setValue(pwd);
    }

    statusMsg_ = "Hosting LAN match...";
    BMLOG("WML Network: Spawning active listen socket instance on map reference: " + mapPath);

    std::string cmd = "open \"" + mapPath + "\"?listen";
    if (!pwd.empty()) {
        cmd += "?password=" + pwd;
    }

    if (gameWrapper) {
        gameWrapper->ExecuteUnrealCommand(cmd);
    }
}

void WorkshopMapLoader::Render() {
    if (!isWindowOpen_) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(580, 480), ImGuiCond_FirstUseEver);
    
    if (!ImGui::Begin(GetPluginName().c_str(), &isWindowOpen_, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    ImGui::Text("Map Directory Path:");
    ImGui::PushItemWidth(-140.0f); 
    ImGui::InputText("##dir_input_field", mapDirBuf_, sizeof(mapDirBuf_));
    ImGui::PopItemWidth();
    
    ImGui::SameLine();
    if (ImGui::Button("Scan / Refresh", ImVec2(120, 0))) {
        ScanForMaps();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Search Filter:");
    ImGui::PushItemWidth(-1.0f); 
    ImGui::InputText("##search_filter_field", searchBuf_, sizeof(searchBuf_));
    ImGui::PopItemWidth();

    ImGui::Spacing();

    if (ImGui::BeginChild("MapSelectionListContainer", ImVec2(0, 220), true)) {
        std::string searchStr(searchBuf_);
        std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

        for (int i = 0; i < (int)maps_.size(); ++i) {
            std::string mapNameLower = maps_[i].name;
            std::transform(mapNameLower.begin(), mapNameLower.end(), mapNameLower.begin(), ::tolower);

            if (!searchStr.empty() && mapNameLower.find(searchStr) == std::string::npos) {
                continue;
            }

            const bool isSelected = (selectedMapIdx_ == i);
            if (ImGui::Selectable(maps_[i].name.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
                selectedMapIdx_ = i;
                
                if (ImGui::IsMouseDoubleClicked(0)) {
                    EnterMap(maps_[i].path);
                }
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndChild();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const bool clearToRun = (selectedMapIdx_ >= 0 && selectedMapIdx_ < (int)maps_.size());

    if (!clearToRun) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.35f);
    }

    if (ImGui::Button("Enter Map", ImVec2(160, 45))) {
        if (clearToRun) {
            EnterMap(maps_[selectedMapIdx_].path);
        }
    }
    
    if (!clearToRun) {
        ImGui::PopStyleVar();
    }

    ImGui::SameLine(0, 35);

    ImGui::BeginGroup();
    ImGui::Text("LAN Room Password (Optional):");
    ImGui::SetNextItemWidth(180);
    ImGui::InputText("##lan_room_password_field", lanPasswordBuf_, sizeof(lanPasswordBuf_));
    
    ImGui::Spacing();

    if (!clearToRun) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.35f);
    }
    
    if (ImGui::Button("Create LAN Match", ImVec2(180, 0))) {
        if (clearToRun) {
            CreateLanMatch();
        }
    }
    
    if (!clearToRun) {
        ImGui::PopStyleVar();
    }
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("System Status:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0f, 0.75f, 1.0f, 1.0f), statusMsg_.c_str());

    ImGui::End();
}
