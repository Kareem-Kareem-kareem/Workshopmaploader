#pragma once

// Framework Header Inclusions
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"

// Standard Library Implementations
#include <string>
#include <vector>
#include <filesystem>
#include <memory>

/**
 * @brief Representation of a verified custom map file discovered on disk.
 */
struct WorkshopMap {
    std::string name;     // Sanitized presentation name of the target map
    std::string path;     // Fully expanded absolute filesystem pathway address
};

/**
 * @brief Principal controller class managing the execution lifecycle, UI rendering,
 * and engine level commands for custom map virtualization.
 */
class WorkshopMapLoader : public BakkesModPlugin, public PluginWindow {
public:
    //-----------------------------------------------------------------
    // Core Plugin Lifecycle Hooks
    //-----------------------------------------------------------------
    /**
     * @brief Executed instantly when the plugin context is loaded into engine memory.
     */
    void onLoad() override;

    /**
     * @brief Executed cleanly when the plugin context is dismantled or unloaded by the host.
     */
    void onUnload() override;

    //-----------------------------------------------------------------
    // ImGui Interface Subsystem Overrides
    //-----------------------------------------------------------------
    /**
     * @brief Main processing loops for structural canvas composition and UI node evaluation.
     */
    void Render() override;

    /**
     * @brief Returns the strict identifier tracking key mapped to the interface canvas.
     */
    std::string GetPluginName() override;

    /**
     * @brief Assigns the active global ImGui rendering instance handle across execution bounds.
     * @param ctx Opaque pointer referencing the active ImGuiContext structure allocation.
     */
    void SetImGuiContext(uintptr_t ctx) override;

    //-----------------------------------------------------------------
    // Interface Panel Visibility Event Overrides
    //-----------------------------------------------------------------
    /**
     * @brief Event handler triggered when the primary interface viewport transitions to open.
     */
    void OnOpen() override;

    /**
     * @brief Event handler triggered when the primary interface viewport transitions to closed.
     */
    void OnClose() override;

    //-----------------------------------------------------------------
    // Core Functional Logic Subsystems
    //-----------------------------------------------------------------
    /**
     * @brief Scans the configured local operating path recursively for valid custom map structures.
     */
    void ScanForMaps();

    /**
     * @brief Routes training command structures to play standard custom files inside offline parameters.
     * @param mapPath The verified target file string to bind to the level initialization cycle.
     */
    void EnterMap(const std::string& mapPath);

    /**
     * @brief Deploys an offline listen instance mapping local network network configurations.
     */
    void CreateLanMatch();

private:
    // Memory Storage Components
    std::vector<WorkshopMap> maps_;       // Thread-safe tracked array holding validated map entries
    int selectedMapIdx_ = -1;             // Current active selection index within the map collection array
    bool isWindowOpen_ = false;           // State parameter regulating the open/closed flags of the viewport UI
    std::string statusMsg_ = "Ready";     // Dynamic diagnostic trace feedback tracking runtime transactions

    // Input Tracking Buffers
    char searchBuf_[256] = "";            // Search text normalization buffer filtering data lists
    char mapDirBuf_[512] = "";            // Text buffer holding active targets for directory polling pipelines
    char lanPasswordBuf_[128] = "";       // Text buffer managing security keys for hosted network rooms
};
