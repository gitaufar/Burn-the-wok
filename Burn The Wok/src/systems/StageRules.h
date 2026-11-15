#pragma once

#include <algorithm>

namespace game {

enum class StageOutcome {
    Playing,
    Won,       // every fire is out before the time runs out
    LostSawit, // too much of the forest became oil-palm plantation
    LostTime,
    LostAnimals,
};

// Timer and objectives of one stage.
class StageRules {
public:
    // initialTrees = burnable trees at the start; every percentage is relative to it.
    StageRules(float duration, float sawitLoseFraction, int initialTrees, int rescueTarget)
        : m_timeLeft(duration), m_sawitLoseFraction(sawitLoseFraction),
          m_initialTrees(std::max(1, initialTrees)), m_rescueTarget(rescueTarget) {}

    // Advances the timer and decides the outcome. Once decided it never changes.
    // Priority is plantation threshold, impossible rescue quota, timeout, then win.
    // Objectives reached on the timeout tick count as a loss.
    StageOutcome Update(float dt, int burningFires, int sawitTrees, int charredTrees, int rescued, int pending) {
        if (m_outcome != StageOutcome::Playing) return m_outcome;
        m_timeLeft = std::max(0.0f, m_timeLeft - dt);
        m_sawitTrees = sawitTrees;
        m_charredTrees = charredTrees;
        if (GetSawitFraction() >= m_sawitLoseFraction) {
            m_outcome = StageOutcome::LostSawit;
        } else if (rescued + pending < m_rescueTarget) {
            m_outcome = StageOutcome::LostAnimals;
        } else if (m_timeLeft <= 0.0f) {
            m_outcome = StageOutcome::LostTime;
        } else if (burningFires == 0 && rescued >= m_rescueTarget) {
            m_outcome = StageOutcome::Won;
        }
        return m_outcome;
    }

    StageOutcome GetOutcome() const { return m_outcome; }
    float GetTimeLeft() const { return m_timeLeft; }
    int GetInitialTrees() const { return m_initialTrees; }
    float GetSawitFraction() const { return static_cast<float>(m_sawitTrees) / m_initialTrees; }
    float GetSawitLoseFraction() const { return m_sawitLoseFraction; }
    // Share of the starting forest that is still forest (burning trees still count).
    float GetForestFraction() const { return 1.0f - static_cast<float>(m_sawitTrees + m_charredTrees) / m_initialTrees; }

private:
    float m_timeLeft;
    float m_sawitLoseFraction;
    int m_initialTrees;
    int m_rescueTarget;
    int m_sawitTrees = 0;
    int m_charredTrees = 0;
    StageOutcome m_outcome = StageOutcome::Playing;
};

} // namespace game
