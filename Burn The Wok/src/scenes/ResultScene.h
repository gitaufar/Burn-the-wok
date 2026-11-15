#pragma once

#include "GameConfig.h"
#include "audio/SoundBank.h"

#include <engine/core/Scene.h>

#include <memory>

#include <string>
#include <utility>

namespace game {

// Win ("Menang") or lose ("Kalah") screen with the reason and the stage numbers.
// Placeholder visuals until the UI system and star rating arrive in Stage 6.
// Enter/Space continues the campaign or retries. Esc returns to the menu.
class ResultScene : public engine::Scene {
public:
    enum class Outcome { Win, Lose };

    // What to show under the title. Uppercase only (the pixel font has no lowercase).
    struct Summary {
        std::string reason;     // e.g. "SEMUA API PADAM"; empty = no reason line
        bool hasStats = false;  // false for the F1/F2 debug shortcuts
        int forestPercent = 0;  // starting forest still standing
        int sawitPercent = 0;
        int firesPutOut = 0;
        int secondsLeft = 0;
        config::StageId stage = config::StageId::FirstFire;
        int rescued = 0;
        int rescueTarget = 0;
    };

    explicit ResultScene(Outcome outcome) : ResultScene(outcome, Summary{}) {}
    ResultScene(Outcome outcome, Summary summary)
        : m_outcome(outcome), m_summary(std::move(summary)) {}

    void OnEnter() override;
    void Update(float dt) override;
    void Draw(float alpha) override;

private:
    const char* ActionHint() const;
    Outcome m_outcome;
    Summary m_summary;
    std::unique_ptr<SoundBank> m_sfx;
};

} // namespace game
