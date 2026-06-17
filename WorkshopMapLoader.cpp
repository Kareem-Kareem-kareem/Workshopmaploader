#include "WorkshopMapLoader.h"

BAKKESMOD_PLUGIN(WorkshopMapLoader, "Simple Test Plugin", "1.0", PLUGINTYPE_FREEPLAY)

void WorkshopMapLoader::onLoad() {
    cvarManager->log("Plugin loaded successfully!");
}

void WorkshopMapLoader::onUnload() {
}
