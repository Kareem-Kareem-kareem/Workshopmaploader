#include "WorkshopMapLoader.h"
#include "bakkesmod/wrappers/GameEvent/ServerWrapper.h"
#include "bakkesmod/wrappers/GameObject/CarWrapper.h"
#include "bakkesmod/wrappers/GameObject/PRIWrapper.h"
#include "bakkesmod/wrappers/canvaswrapper.h"

using namespace BakkesMod::Plugin;

BAKKESMOD_PLUGIN(WorkshopMapLoader, "Live Session Stat Tracker", "1.0", PLUGINTYPE_FREEPLAY)

void WorkshopMapLoader::onLoad() {
    if (!gameWrapper) return;

    gameWrapper->RegisterDrawFunc(std::bind(&WorkshopMapLoader::RenderCanvas, this, std::placeholders::_1));
}

void WorkshopMapLoader::onUnload() {
}

void WorkshopMapLoader::RenderCanvas(CanvasWrapper canvas) {
    ServerWrapper server = gameWrapper->GetCurrentGameState();
    if (!server) return;

    CarWrapper localCar = gameWrapper->GetLocalCar();
    if (!localCar) return;

    PRIWrapper pri = localCar.GetPRI();
    if (!pri) return;

    goals_ = pri.GetMatchGoals();
    saves_ = pri.GetMatchSaves();
    shots_ = pri.GetMatchShots();
    assists_ = pri.GetMatchAssists();

    auto boostComponent = localCar.GetBoostComponent();
    if (boostComponent) {
        boostAmount_ = boostComponent.GetCurrentBoostAmount() * 100.0f;
    }

    canvas.SetPosition(LinearColor{ 0, 0, 0, 150 });
    canvas.SetColor(0, 0, 0, 150);
    canvas.FillBox(Vector2{ 20, 20 }, Vector2{ 220, 160 });

    canvas.SetColor(255, 255, 255, 255);
    
    canvas.SetPosition(Vector2{ 35, 30 });
    canvas.DrawString("SESSION LIVE STATS", 1.2f, 1.2f);

    canvas.SetPosition(Vector2{ 35, 60 });
    canvas.DrawString("Goals: " + std::to_string(goals_), 1.0f, 1.0f);

    canvas.SetPosition(Vector2{ 35, 85 });
    canvas.DrawString("Saves: " + std::to_string(saves_), 1.0f, 1.0f);

    canvas.SetPosition(Vector2{ 35, 110 });
    canvas.DrawString("Shots: " + std::to_string(shots_), 1.0f, 1.0f);

    canvas.SetPosition(Vector2{ 35, 135 });
    canvas.DrawString("Assists: " + std::to_string(assists_), 1.0f, 1.0f);

    if (boostAmount_ < 25.0f) {
        canvas.SetColor(255, 50, 50, 255);
    } else {
        canvas.SetColor(50, 255, 50, 255);
    }
    canvas.SetPosition(Vector2{ 35, 160 });
    canvas.DrawString("Live Boost: " + std::to_string(static_cast<int>(boostAmount_)) + "%", 1.0f, 1.0f);
}
