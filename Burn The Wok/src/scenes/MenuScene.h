#pragma once

#include <engine/core/Scene.h>

namespace game {

// Main menu. No text rendering yet (UI system is Stage 6), so the controls are shown
// in the window title. Enter/Space = start, Esc = quit.
class MenuScene : public engine::Scene {
public:
    void OnEnter() override;
    void Update(float dt) override;
    void Draw(float alpha) override;

private:
    float m_time = 0.0f;
};

} // namespace game
