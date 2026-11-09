#pragma once

#include "entities/Player.h"
#include "world/ForestMap.h"

#include <engine/core/Scene.h>
#include <engine/graphics/Camera2D.h>
#include <engine/graphics/Texture.h>

#include <memory>

namespace game {

// Stage 1 demo: the ranger walks around a placeholder forest map.
// WASD/arrows = move, mouse wheel = zoom, F1/F2 = jump to win/lose screen (debug),
// F3 = show hitboxes and solid tiles (debug),
// Esc = back to the menu (a real pause screen comes in Stage 6).
class GameplayScene : public engine::Scene {
public:
    GameplayScene();

    void OnEnter() override;
    void Update(float dt) override;
    void Draw(float alpha) override;

private:
    void UpdateCamera(float alpha);
    void DrawOverlay(float alpha);
    void DrawCollisionDebug(float alpha); // inside the world batch (camera space)

    std::unique_ptr<engine::Texture> m_atlas; // created in OnEnter (needs the GL context)
    std::unique_ptr<engine::Texture> m_treeTexture;
    std::unique_ptr<engine::Texture> m_minimap;
    std::unique_ptr<engine::Texture> m_vignette;
    engine::Rect m_treeSource;
    ForestMap m_map;
    Player m_player;
    engine::Camera2D m_camera;
    int m_zoom;
    float m_time = 0.0f;
    bool m_showCollision = false; // F3
};

} // namespace game
