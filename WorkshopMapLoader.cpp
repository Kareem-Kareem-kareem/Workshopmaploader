#include "WorkshopMapLoader.h"
#include "bakkesmod/wrappers/GameEvent/ServerWrapper.h"
#include "bakkesmod/wrappers/GameObject/BallWrapper.h"
#include "bakkesmod/wrappers/GameObject/CarWrapper.h"
#include "bakkesmod/wrappers/GameObject/CVarWrapper.h"

using namespace BakkesMod::Plugin;

BAKKESMOD_PLUGIN(WorkshopMapLoader, "Ball Auto Aim Mod", "1.0", PLUGINTYPE_FREEPLAY)

void WorkshopMapLoader::onLoad() {
    if (!cvarManager) return;

    gameWrapper->HookEvent("TAGame.Car_TA.EventHitBall", std::bind(&WorkshopMapLoader::OnBallHit, this, std::placeholders::_1));
    gameWrapper->HookEvent("TAGame.GameEvent_Soccer_TA.OnPhysicsStep", std::bind(&WorkshopMapLoader::OnPhysicsTick, this, std::placeholders::_1));
}

void WorkshopMapLoader::onUnload() {
    gameWrapper->UnhookEvent("TAGame.Car_TA.EventHitBall");
    gameWrapper->UnhookEvent("TAGame.GameEvent_Soccer_TA.OnPhysicsStep");
}

void WorkshopMapLoader::OnBallHit(std::string eventName) {
    ServerWrapper server = gameWrapper->GetCurrentGameState();
    if (!server) return;

    BallWrapper ball = server.GetBall();
    if (!ball) return;

    CarWrapper localCar = gameWrapper->GetLocalCar();
    if (!localCar) return;

    auto cars = server.GetCars();
    for (int i = 0; i < cars.Count(); ++i) {
        CarWrapper car = cars.Get(i);
        if (!car) continue;

        if (car.GetBallHitInfo().bHitBall) {
            if (car.GetPRI() && localCar.GetPRI() && car.GetPRI().GetUniqueId() == localCar.GetPRI().GetUniqueId()) {
                isAutoAimActive_ = true;
            } else {
                isAutoAimActive_ = false;
            }
            break;
        }
    }
}

void WorkshopMapLoader::OnPhysicsTick(std::string eventName) {
    if (!isAutoAimActive_) return;

    ServerWrapper server = gameWrapper->GetCurrentGameState();
    if (!server) return;

    BallWrapper ball = server.GetBall();
    if (!ball) return;

    CarWrapper localCar = gameWrapper->GetLocalCar();
    if (!localCar) return;

    unsigned char localTeam = localCar.GetTeamNum2();
    Vector goalLocation(0, 5120, 0);

    if (localTeam == 0) {
        goalLocation.Y = 5120;
    } else {
        goalLocation.Y = -5120;
    }

    Vector ballLocation = ball.GetLocation();
    Vector targetDirection = goalLocation - ballLocation;
    
    float distance = sqrt(targetDirection.X * targetDirection.X + targetDirection.Y * targetDirection.Y + targetDirection.Z * targetDirection.Z);
    if (distance > 0.1f) {
        targetDirection.X /= distance;
        targetDirection.Y /= distance;
        targetDirection.Z /= distance;
    }

    Vector ballVelocity = ball.GetVelocity();
    float currentSpeed = sqrt(ballVelocity.X * ballVelocity.X + ballVelocity.Y * ballVelocity.Y + ballVelocity.Z * ballVelocity.Z);

    if (currentSpeed < 500.0f) {
        currentSpeed = 1500.0f;
    }

    Vector newVelocity;
    newVelocity.X = targetDirection.X * currentSpeed;
    newVelocity.Y = targetDirection.Y * currentSpeed;
    newVelocity.Z = targetDirection.Z * currentSpeed;

    ball.SetVelocity(newVelocity);
}
