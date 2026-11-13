#pragma once

#include <algorithm>

namespace game {

enum class StageOutcome {
    Playing,
    Won,       // every fire is out before the time runs out
    LostSawit, // too much of the forest became oil-palm plantation
    LostTime,  // the time ran out while fires were still burning
};

// Timer and win/lose rules of one stage. Pure logic: the scene feeds it the current
// numbers each update and reacts to the outcome. Rescuing animals joins these rules in
// Stage 4 (docs/03-gameplay-design.md, "Kondisi menang & kalah").
class StageRules {
public:
    // initialTrees = burnable trees at the start; every percentage is relative to it.
    StageRules(float duration, float sawitLoseFraction, int initialTrees)
        : m_timeLeft(duration), m_sawitLoseFraction(sawitLoseFraction),
          m_initialTrees(std::max(1, initialTrees)) {}

    // Advances the timer and decides the outcome. Once decided it never changes.
    // Order matters: losing land is checked before "all fires out", so the fire that
    // turns the last allowed tree into sawit cannot also count as a win.
    StageOutcome Update(float dt, int burningFires, int sawitTrees) {
        if (m_outcome != StageOutcome::Playing) return m_outcome;
        m_timeLeft = std::max(0.0f, m_timeLeft - dt);
        m_sawitTrees = sawitTrees;
        if (GetSawitFraction() >= m_sawitLoseFraction) {
            m_outcome = StageOutcome::LostSawit;
        } else if (burningFires == 0) {
            m_outcome = StageOutcome::Won;
        } else if (m_timeLeft <= 0.0f) {
            m_outcome = StageOutcome::LostTime;
        }
        return m_outcome;
    }

    StageOutcome GetOutcome() const { return m_outcome; }
    float GetTimeLeft() const { return m_timeLeft; }
    int GetInitialTrees() const { return m_initialTrees; }
    float GetSawitFraction() const { return static_cast<float>(m_sawitTrees) / m_initialTrees; }
    float GetSawitLoseFraction() const { return m_sawitLoseFraction; }
    // Share of the starting forest that is still forest (burning trees still count).
    float GetForestFraction() const { return 1.0f - GetSawitFraction(); }

private:
    float m_timeLeft;
    float m_sawitLoseFraction;
    int m_initialTrees;
    int m_sawitTrees = 0;
    StageOutcome m_outcome = StageOutcome::Playing;
};

} // namespace game
