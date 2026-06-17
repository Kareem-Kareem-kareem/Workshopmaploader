#include "WorkshopMapLoader.h"
#include "imgui/imgui.h"
#include <fstream>
#include <algorithm>
#include <cctype>

// Register the core plugin instantiation node with the underlying BakkesMod framework ecosystem
BAKKESMOD_PLUGIN(WorkshopMapLoader, "Workshop Map Loader", "2.0", PLUGINTYPE_FREEPLAY)

/**
 * @brief High-reliability trace logger macro protecting engine calls from null interface initializations.
 */
#define LOGGER(msg) if (cvarManager) { cvarManager->log("WML_LOG: " + std::string(msg)); }

void WorkshopMapLoader::onLoad() {
    // Immediate early-exit defensive guard check against framework pointer degradation
    if (!cvarManager) return;

    // Register primary persistent configuration values within the environment database
    cvarManager->registerCvar("wml_directory", "C:\\Program Files (x86)\\Steam\\steamapps\\workshop\\content\\252950", "Default Workshop Maps Directory Path", true, false, 0, false, 0, true);
    cvarManager->registerCvar("wml_password", "", "LAN Server Room Password Protection Key", true, false, 0, false, 0, true);

    // Hydrate persistent buffer memories from stored configuration files safely
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

    // Execute first boot discovery indexing run sequence
    ScanForMaps();

    // Register accessible console command interfaces inside the engine developer window
    cvarManager->registerNotifier("wml_scan", [this](std::vector<std::string> args) {
        ScanForMaps();
    }, "Triggers an immediate recursive scanning pass over the targeted file directory path", PERMISSION_ALL);

    cvarManager->registerNotifier("wml_enter", [this](std::vector<std::string> args) {
        if (selectedMapIdx_ >= 0 && selectedMapIdx_ < static_cast<int>(maps_.size())) {
            EnterMap(maps_[selectedMapIdx_].path);
        } else {
            statusMsg_ = "Action Failed: No valid map selection has been established.";
            LOGGER("Execution error: Command 'wml_enter' rejected due to an empty index focus parameter.");
        }
    }, "Loads the active selected map entry directly into offline local environment context parameters", PERMISSION_ALL);

    cvarManager->registerNotifier("wml_toggle", [this](std::vector<std::string> args) {
        isWindowOpen_ = !isWindowOpen_;
        if (isWindowOpen_) {
            OnOpen();
        } else {
            OnClose();
        }
        LOGGER("Viewport status toggled smoothly via native console command notification context.");
    }, "Toggles the operational visibility state parameters of the plugin panel view canvas overlay", PERMISSION_ALL);

    LOGGER("Initialization pass executed flawlessly. Module is standing by.");
}

void WorkshopMapLoader::onUnload() {
    LOGGER("Unloading sequence initialized. De-allocating references and cleaning execution tracking states.");
}

void WorkshopMapLoader::SetImGuiContext(uintptr_t ctx) {
    ImGuiContext* localContext = reinterpret_cast<ImGuiContext*>(ctx);
    ImGui::SetCurrentContext(localContext);
    LOGGER("Successfully bound incoming global context references to internal ImGui tracking modules.");
}

std::string WorkshopMapLoader::GetPluginName() {
    return "Workshop Map Loader Pro";
}

void WorkshopMapLoader::OnOpen() {
    isWindowOpen = true;
    LOGGER("Plugin control layout display window toggled to visible.");
}

void WorkshopMapLoader::OnClose() {
    isWindowOpen = false;
    LOGGER("Plugin control layout display window toggled to hidden.");
}

void WorkshopMapLoader::ScanForMaps() {
    maps_.clear();
    selectedMapIdx_ = -1;
    std::string pathParserString(mapDirBuf_);

    // Path Hygiene Pipeline: Strip out loose escape quotation wraps accidentally copied from systems
    pathParserString.erase(std::remove(pathParserString.begin(), pathParserString.end(), '\"'), pathParserString.end());
    while (!pathParserString.empty() && std::isspace(static_cast<unsigned char>(pathParserString.back()))) {
        pathParserString.pop_back();
    }

    if (pathParserString.empty()) {
        statusMsg_ = "Error: Configured target system path string evaluates to blank.";
        return;
    }

    if (!std::filesystem::exists(pathParserString)) {
        statusMsg_ = "Error: Directory target does not exist. Check file path string precision.";
        LOGGER("Directory missing warning: The address '" + pathParserString + "' could not be evaluated by the filesystem.");
        return;
    }

    try {
        if (std::filesystem::is_directory(pathParserString)) {
            // Traverse directory hierarchy recursively tracking across application layer thresholds safely
            for (const auto& entryItem : std::filesystem::recursive_directory_iterator(pathParserString, std::filesystem::directory_options::skip_permission_denied)) {
                try {
                    if (entryItem.is_regular_file()) {
                        std::string activeExtensionString = entryItem.path().extension().string();
                        // Standard normalization to lower context formatting to block case mismatch failures
                        std::transform(activeExtensionString.begin(), activeExtensionString.end(), activeExtensionString.begin(), ::tolower);

                        if (activeExtensionString == ".udk" || activeExtensionString == ".upk") {
                            WorkshopMap indexedMapRecord;
                            indexedMapRecord.name = entryItem.path().stem().string();
                            indexedMapRecord.path = entryItem.path().string();
                            maps_.push_back(indexedMapRecord);
                        }
                    }
                }
                catch (const std::exception& innerFileError) {
                    LOGGER("File system item read exception encountered (Skipping element): " + std::string(innerFileError.what()));
                    continue;
                }
            }

            statusMsg_ = "Scan Completed: Successfully indexed " + std::to_string(maps_.size()) + " custom files.";
            LOGGER("Scanning transaction successful. Local structure records updated cleanly.");

            if (cvarManager) {
                cvarManager->getCvar("wml_directory").setValue(pathParserString);
            }
        } else {
            statusMsg_ = "Error: The configured location points to a file, not a workspace directory folder.";
        }
    }
    catch (const std::exception& globalFsException) {
        statusMsg_ = "Critical Index Fault: " + std::string(globalFsException.what());
        LOGGER("Core framework system IO error tracked: " + std::string(globalFsException.what()));
    }
}

void WorkshopMapLoader::EnterMap(const std::string& mapPath) {
    if (mapPath.empty()) {
        statusMsg_ = "Error: Requested launch tracking path points to an invalid string location.";
        return;
    }

    statusMsg_ = "Executing level load command sequences...";
    LOGGER("Routing environment context mapping. Invoking engine training map sequence for: " + mapPath);

    if (gameWrapper) {
        // Safe dispatching of standard map streaming functions within offline parameters
        gameWrapper->GetGfxTrainingData().PlayFreeplayMap(mapPath);
    } else {
        LOGGER("Critical Runtime Exception: Cannot load level map. Game wrapper handle instance is uninstantiated.");
    }
}

void WorkshopMapLoader::CreateLanMatch() {
    if (selectedMapIdx_ < 0 || selectedMapIdx_ >= static_cast<int>(maps_.size())) {
        statusMsg = "Error: An indexed map target must be active inside the viewport array panel first.";
        return;
    }

    std::string localizedMapTargetAddress = maps_[selectedMapIdx_].path;
    std::string securityTokenString(lanPasswordBuf_);

    if (cvarManager) {
        cvarManager->getCvar("wml_password").setValue(securityTokenString);
    }

    statusMsg_ = "Spawning network server instance loops...";
    LOGGER("Network orchestration initiated. Deploying map listen layer over file source path target.");

    // Format strict Unreal terminal map strings safely: open "[Path]"?listen?password=[Value]
    std::string executionTerminalCommandString = "open \"" + localizedMapTargetAddress + "\"?listen";
    if (!securityTokenString.empty()) {
        executionTerminalCommandString += "?password=" + securityTokenString;
    }

    if (gameWrapper) {
        gameWrapper->ExecuteUnrealCommand(executionTerminalCommandString);
    } else {
        LOGGER("Network failure: Core engine wrapper uninstantiated during active network pass.");
    }
}

void WorkshopMapLoader::Render() {
    if (!isWindowOpen_) {
        return;
    }

    // Assign consistent default proportions during initialization view setups
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin(GetPluginName().c_str(), &isWindowOpen_, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    //-----------------------------------------------------------------
    // Section A: Directory Track Management Options
    //-----------------------------------------------------------------
    ImGui::Text("Target Custom Maps Folder Pathway:");
    ImGui::PushItemWidth(-150.0f);
    ImGui::InputText("##directory_path_input_field", mapDirBuf_, sizeof(mapDirBuf_));
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (ImGui::Button("Index Path Target", ImVec2(130, 0))) {
        ScanForMaps();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    //-----------------------------------------------------------------
    // Section B: Real-Time Search Filtering Canvas
    //-----------------------------------------------------------------
    ImGui::Text("Filter Maps By String Name:");
    ImGui::PushItemWidth(-1.0f);
    ImGui::InputText("##search_filtration_input_field", searchBuf_, sizeof(searchBuf_));
    ImGui::PopItemWidth();

    ImGui::Spacing();

    //-----------------------------------------------------------------
    // Section C: Structured Map Catalog Hierarchy
    //-----------------------------------------------------------------
    if (ImGui::BeginChild("StructuredCatalogViewPanel", ImVec2(0, 240), true)) {
        std::string operationalSearchString(searchBuf_);
        std::transform(operationalSearchString.begin(), operationalSearchString.end(), operationalSearchString.begin(), ::tolower);

        for (int indexLoopTracker = 0; indexLoopTracker < static_cast<int>(maps_.size()); ++indexLoopTracker) {
            std::string normalizedMapRecordName = maps_[indexLoopTracker].name;
            std::transform(normalizedMapRecordName.begin(), normalizedMapRecordName.end(), normalizedMapRecordName.begin(), ::tolower);

            // Discard rendering item entries that completely fall outside criteria parameters
            if (!operationalSearchString.empty() && normalizedMapRecordName.find(operationalSearchString) == std::string::npos) {
                continue;
            }

            const bool selectionHighlightFlag = (selectedMapIdx_ == indexLoopTracker);
            if (ImGui::Selectable(maps_[indexLoopTracker].name.c_str(), selectionHighlightFlag, ImGuiSelectableFlags_AllowDoubleClick)) {
                selectedMapIdx_ = indexLoopTracker;

                // Double click interaction layer acts as a shortcut execution trigger
                if (ImGui::IsMouseDoubleClicked(0)) {
                    EnterMap(maps_[indexLoopTracker].path);
                }
            }

            if (selectionHighlightFlag) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndChild();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    //-----------------------------------------------------------------
    // Section D: Launch Mechanics Control Centers
    //-----------------------------------------------------------------
    const bool isSelectionValidAndConfirmed = (selectedMapIdx_ >= 0 && selectedMapIdx_ < static_cast<int>(maps_.size()));

    // Attenuate UI opacity states cleanly if active buttons are non-interactable
    if (!isSelectionValidAndConfirmed) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.40f);
    }

    if (ImGui::Button("Launch Offline Freeplay", ImVec2(200, 45))) {
        if (isSelectionValidAndConfirmed) {
            EnterMap(maps_[selectedMapIdx_].path);
        }
    }

    if (!isSelectionValidAndConfirmed) {
        ImGui::PopStyleVar();
    }

    ImGui::SameLine(0, 40);

    ImGui::BeginGroup();
    ImGui::Text("Network Access Key (LAN):");
    ImGui::SetNextItemWidth(180);
    ImGui::InputText("##lan_network_room_security_key", lanPasswordBuf_, sizeof(lanPasswordBuf_));

    ImGui::Spacing();

    if (!isSelectionValidAndConfirmed) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.40f);
    }

    if (ImGui::Button("Host Local LAN Match", ImVec2(180, 0))) {
        if (isSelectionValidAndConfirmed) {
            CreateLanMatch();
        }
    }

    if (!isSelectionValidAndConfirmed) {
        ImGui::PopStyleVar();
    }
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    //-----------------------------------------------------------------
    // Section E: Diagnostics / Pipeline System Tracking Feed
    //-----------------------------------------------------------------
    ImGui::Text("System Trace Status:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.1f, 0.8f, 1.0f, 1.0f), statusMsg_.c_str());

    ImGui::End();
}
