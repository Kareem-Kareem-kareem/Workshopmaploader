#include "WorkshopMapLoader.h"
#include "bakkesmod/wrappers/GameObject/CVarManagerWrapper.h"

using namespace BakkesMod::Plugin;

BAKKESMOD_PLUGIN(WorkshopMapLoader, "Camera Preset Toggle", "1.0", PLUGINTYPE_FREEPLAY)

void WorkshopMapLoader::onLoad() {
    if (!cvarManager) return;

    cvarManager->registerNotifier("cam_toggle_view", [this](std::vector<std::string> args) {
        ToggleCameraSettings();
    }, "Toggles between normal and wide camera presets", PERMISSION_ALL);
}

void WorkshopMapLoader::onUnload() {
}

void WorkshopMapLoader::ToggleCameraSettings() {
    if (!cvarManager) return;

    auto distanceCvar = cvarManager->getCvar("cl_camera_distance");
    auto fovCvar = cvarManager->getCvar("cl_camera_fov");
    auto heightCvar = cvarManager->getCvar("cl_camera_height");

    if (!distanceCvar || !fovCvar || !heightCvar) return;

    if (!isWideViewActive_) {
        distanceCvar.setValue(400.0f);
        fovCvar.setValue(110.0f);
        heightCvar.setValue(150.0f);
        isWideViewActive_ = true;
    } else {
        distanceCvar.setValue(270.0f);
        fovCvar.setValue(110.0f);
        heightCvar.setValue(110.0f);
        isWideViewActive_ = false;
    }
}
