#pragma once

#include "GameConfig.h"
#include "systems/PointCalculator.h"
#include "audio/SoundBank.h"

#include <engine/core/Scene.h>
#include <engine/graphics/Texture.h>
#include <engine/math/Rect.h>

#include <memory>

#include <string>
#include <utility>

namespace game {

// Win ("Menang") or lose ("Kalah") screen with the reason and the stage numbers.
// Field report with mouse buttons and keyboard shortcuts.
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
        int totalAnimals = 0;
        int wildlifePoints = 0;
        float campaignTime = 0;
        int stars = 0;
        PointBreakdown points;
    };

    explicit ResultScene(Outcome outcome) : ResultScene(outcome, Summary{}) {}
    ResultScene(Outcome outcome, Summary summary)
        : m_outcome(outcome), m_summary(std::move(summary)) {}

    void OnEnter() override;
    void Update(float dt) override;
    void Draw(float alpha) override;

private:
    const char* ActionHint() const;
    const char* ActionLabel() const;
    bool ReturnsToMenu() const;
    void Activate(bool menu);
    Outcome m_outcome;
    Summary m_summary;
    std::unique_ptr<SoundBank> m_sfx;
    std::unique_ptr<engine::Texture> m_tree, m_ranger;
    engine::Rect m_treeSource, m_rangerSource;
    float m_time = 0.0f;
    int m_hovered = 0;
    bool m_saveFailed = false;
    bool m_storeUnlocked = false;
    bool m_recorded = false;
    int m_balance = 0;
};

} // namespace game
