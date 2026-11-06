#pragma once

#include <engine/core/Scene.h>

namespace game {

// Win ("Menang") or lose ("Kalah") screen. Placeholder visuals until the UI
// system and star rating arrive in Stage 6. Enter/Space = back to the menu.
class ResultScene : public engine::Scene {
public:
    enum class Outcome { Win, Lose };

    explicit ResultScene(Outcome outcome) : m_outcome(outcome) {}

    void OnEnter() override;
    void Update(float dt) override;
    void Draw(float alpha) override;

private:
    Outcome m_outcome;
};

} // namespace game
