#pragma once

#include "GameConfig.h"

namespace game {
struct PointBreakdown {
    int clear = 0, wildlife = 0, stars = 0, time = 0, perfect = 0;
    int Total() const { return clear + wildlife + stars + time + perfect; }
};

constexpr int RescuePoints(config::ActorKind kind) {
    return kind == config::ActorKind::Owl ? 35 : 25;
}

constexpr int StarRating(bool won, int forestPercent, int rescued, int total) {
    if (!won) return 0;
    return 1 + (forestPercent >= 90 ? 1 : 0) + (rescued == total ? 1 : 0);
}

constexpr PointBreakdown CalculatePoints(config::StageId stage, bool won, bool replay,
    int rescued, int wildlife, int stars, int secondsLeft) {
    if (!won) return {0, wildlife / 2};
    const int reward = 100 + 50 * static_cast<int>(stage);
    return {replay ? reward / 2 : reward, wildlife, stars * 25, secondsLeft / 2,
            rescued == config::GetStage(stage).animalCount ? 50 : 0};
}
}
