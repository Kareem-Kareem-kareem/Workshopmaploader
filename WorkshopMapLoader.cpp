#include "WorkshopMapLoader.h"

using namespace BakkesMod::Plugin;

BAKKESMOD_PLUGIN(WorkshopMapLoader, "Simple Test Plugin", "1.0", PLUGINTYPE_FREEPLAY)

void WorkshopMapLoader::onLoad() {
    if (!cvarManager) return;

    cvarManager->log("Simple Test Plugin successfully loaded into the game!");

    cvarManager->registerNotifier("test_hello", [this](std::vector<std::string> args) {
        if (cvarManager) {
            cvarManager->log("Hello from the plugin console command!");
        }
    }, "Prints a confirmation message to the log terminal", PERMISSION_ALL);
}

void WorkshopMapLoader::onUnload() {
}
