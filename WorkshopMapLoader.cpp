#include "WorkshopMapLoader.h"

BAKKESMOD_PLUGIN(WorkshopMapLoader, "test", "1.0", PLUGINTYPE_FREEPLAY)

void WorkshopMapLoader::onLoad() {
    cvarManager->log("Plugin 'test' loaded successfully!");
}

void WorkshopMapLoader::onUnload() {
}
