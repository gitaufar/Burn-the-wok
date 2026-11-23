#pragma once

#include "GameConfig.h"

namespace game {
enum Upgrade { Tank, Nozzle, Pump, Boots, Crew, Local, National, UpgradeCount };

struct Stock {
    const char* title;
    const char* key;
    const char* description;
    std::array<int, 3> prices;
    int maxLevel;
};

inline constexpr Stock Supplies[] = {
    {"TANGKI BESAR", "upgrade_tank", "BAWA LEBIH BANYAK AIR.", {120,220,350}, 3},
    {"NOZEL TURBO", "upgrade_nozzle", "DAYA SEMPROT +15% PER LEVEL.", {150,280}, 2},
    {"POMPA CEPAT", "upgrade_pump", "ISI ULANG +40% PER LEVEL.", {100,200}, 2},
    {"SEPATU LAPANGAN", "upgrade_boots", "LANGKAH +10% PER LEVEL.", {120,240}, 2},
    {"RELAWAN TETAP", "volunteers", "SATU PETUGAS UNTUK TIAP PATROLI.", {400,700}, 2},
    {"KAMPANYE LOKAL", "local", "PANGGIL 2 RELAWAN / 30 DETIK.", {150}, 1},
    {"KAMPANYE NASIONAL", "national", "PANGGIL 3 RELAWAN / 45 DETIK.", {300}, 1},
};
inline constexpr float TankSizes[] = {config::TankCapacity, 125, 150, 180};
inline constexpr int CampaignSeconds = 600;
}
