#pragma once

#include <engine/core/Scene.h>
#include <engine/graphics/Texture.h>
#include <engine/math/Rect.h>

#include <memory>

namespace game {

// Main menu with a small built-in pixel label and a clickable start button.
// Enter/Space/keypad Enter = start, Esc = quit.
class MenuScene : public engine::Scene {
public:
    void OnEnter() override;
    void Update(float dt) override;
    void Draw(float alpha) override;

private:
    std::unique_ptr<engine::Texture> m_logo;
    std::unique_ptr<engine::Texture> m_glow;
    engine::Rect m_logoSource;
    float m_time = 0.0f;
};

} // namespace game
