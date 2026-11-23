#pragma once

#include "progress/Upgrades.h"

#include <string>

namespace game {
struct SaveData {
    int points = 0;
    int unlocked = 0;
    std::array<int, 3> stars{};
    std::array<bool, 3> cleared{};
    std::array<int, Local> upgrades{};
    bool storeNew = false;
    int campaignTier = 0;
    float campaignSeconds = 0;

    static SaveData& Current();
    bool Commit(const SaveData& next);
};

std::string CampaignClock(float campaignSeconds);
}
