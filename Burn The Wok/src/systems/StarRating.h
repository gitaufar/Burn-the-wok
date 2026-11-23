#pragma once

namespace game {
// Completed stages earn one star. Higher ratings require both forest and rescue quality.
inline int CalculateStars(bool won, int forestPercent, int rescued, int totalAnimals) {
    if (!won) return 0;
    if (totalAnimals > 0 && forestPercent >= 95 && rescued >= totalAnimals) return 3;
    if (totalAnimals > 0 && forestPercent >= 85 && rescued * 4 >= totalAnimals * 3) return 2;
    return 1;
}
} // namespace game
