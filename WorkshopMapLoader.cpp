#include "WorkshopMapLoader.h"

BAKKESMOD_PLUGIN(WorkshopMapLoader, "test", "1.0", PLUGINTYPE_FREEPLAY)

void WorkshopMapLoader::onLoad() {
    cvarManager->log("Plugin 'test' loaded successfully!");

    // This creates the variable that the .set file refers to
    cvarManager->registerCvar("test_enabled_var", "0", "Enables the test feature", true, true, 0, true, 1);
}

void WorkshopMapLoader::onUnload() {
}
