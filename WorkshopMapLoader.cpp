#include "WorkshopMapLoader.h"
#include "imgui/imgui.h"
#include <fstream>
#include <algorithm>
#include <exception>

BAKKESMOD_PLUGIN(WorkshopMapLoader, "Workshop Map Loader", "2.0", PLUGINTYPE_FREEPLAY)

#define BMLOG(msg) cvarManager->log(msg)

void WorkshopMapLoader::onLoad() {
    // 1. Register CVars for persistent storage within BakkesMod's config.cfg
    cvarManager->registerCvar("wml_directory", "C:\\Program Files (x86)\\Steam\\steamapps\\workshop\\content\\252950", "Default Workshop Maps Directory", true, false, 0, false, 0, true);
    cvarManager->registerCvar("wml_password", "", "LAN Server Password", true, false, 0, false, 0, true);

    // 2. Fetch stored configurations and safely copy them into local ImGui character arrays
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

    // 3. Run an initial scan to populate the map selection interface automatically
    ScanForMaps();

    // 4. Register Notifiers so all commands are entirely accessible directly from the console
    cvarManager->registerNotifier("wml_scan", [this](std::vector<std::string> args) {
        ScanForMaps();
    }, "Scan the target directory for custom maps instantly", PERMISSION_ALL);

    cvarManager->registerNotifier("wml_enter", [this](std::vector<std::string> args) {
        if (selectedMapIdx_ >= 0 && selectedMapIdx_ < (int)maps_.size()) {
            EnterMap(maps_[selectedMapIdx_].path);
        } else {
            statusMsg_ = "Error: Cannot execute console load. No map selected.";
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

    // Strip trailing whitespaces or quotation marks accidentally pasted by users
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
                        // Support standard extension criteria
                        if (ext == ".udk" || ext == ".upk") {
                            WorkshopMap m;
                            m.name = entry.path().stem().string(); // Fixed typo from .stub() to .stem()
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
            
            // Push active valid value to config data structures to persist state save
            cvarManager->getCvar("wml_directory").setValue(targetDir);
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
    
    // Core execution hook using internal training data structures for seamless map entry
    gameWrapper->GetGfxTrainingData().PlayFreeplayMap(mapPath);
}

void WorkshopMapLoader::CreateLanMatch() {
    if (selectedMapIdx_ < 0 || selectedMapIdx_ >= (int)maps_.size()) {
        statusMsg_ = "Error: Select a map from the listing interface hierarchy first.";
        return;
    }

    std::string mapPath = maps_[selectedMapIdx_].path;
    std::string pwd(lanPasswordBuf_);
    
    // Save target configuration parameters out to disk
    cvarManager->getCvar("wml_password").setValue(pwd);

    statusMsg_ = "Hosting LAN match...";
    BMLOG("WML Network: Spawning active listen socket instance on map reference: " + mapPath);

    // Build standard listen server parameter flags for local socket engine binding
    std::string cmd = "open \"" + mapPath + "\"?listen";
    if (!pwd.empty()) {
        cmd += "?password=" + pwd;
    }

    gameWrapper->ExecuteUnrealCommand(cmd);
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

    // --- SECTION 1: STORAGE TRACKING CONFIGURATION ---
    ImGui::Text("Map Directory Path:"); // Fixed TextTextUnformatted typo
    ImGui::PushItemWidth(-140.0f); // Reserve space for the button element side anchor
    ImGui::InputText("##dir_input_field", mapDirBuf_, sizeof(mapDirBuf_));
    ImGui::PopItemWidth();
    
    ImGui::SameLine();
    if (ImGui::Button("Scan / Refresh", ImVec2(120, 0))) {
        ScanForMaps();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // --- SECTION 2: LIVE TEXT QUERY FILTER SEARCH ---
    ImGui::Text("Search Filter:"); // Fixed TextTextUnformatted typo
    ImGui::PushItemWidth(-1.0f); // Full width search layout integration bar
    ImGui::InputText("##search_filter_field", searchBuf_, sizeof(searchBuf_));
    ImGui::PopItemWidth();

    ImGui::Spacing();

    // --- SECTION 3: SCROLLABLE DYNAMIC LIST ELEMENT VIEW ---
    if (ImGui::BeginChild("MapSelectionListContainer", ImVec2(0, 220), true, ImGuiWindowFlags_VerticalScrollbar)) {
        std::string searchStr(searchBuf_);
        std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

        for (int i = 0; i < (int)maps_.size(); ++i) {
            std::string mapNameLower = maps_[i].name;
            std::transform(mapNameLower.begin(), mapNameLower.end(), mapNameLower.begin(), ::tolower);

            // Filter out items that do not match our query constraint criteria
            if (!searchStr.empty() && mapNameLower.find(searchStr) == std::string::npos) {
                continue;
            }

            const bool isSelected = (selectedMapIdx_ == i);
            if (ImGui::Selectable(maps_[i].name.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
                selectedMapIdx_ = i;
                
                // Track mouse click event states for shortcut configurations
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

    // --- SECTION 4: CONTROL OPERATIONS CONSOLE ACTIONS ---
    const bool clearToRun = (selectedMapIdx_ >= 0 && selectedMapIdx_ < (int)maps_.size());

    // Fallback Manual Faded Alpha Style Override system block for older ImGui variations
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

    // LAN Management Grid Group Frame
    ImGui::BeginGroup();
    ImGui::Text("LAN Room Password (Optional):"); // Fixed TextTextUnformatted typo
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
    
    // --- SECTION 5: FOOTER FEEDBACK CONSOLE METRICS ---
    ImGui::Text("System Status:"); // Fixed TextTextUnformatted typo
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0f, 0.75f, 1.0f, 1.0f), statusMsg_.c_str());

    ImGui::End();
}
