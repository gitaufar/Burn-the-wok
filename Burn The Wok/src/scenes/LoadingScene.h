#pragma once

#include "GameConfig.h"

#include <engine/core/Scene.h>
#include <engine/graphics/Texture.h>
#include <engine/math/Rect.h>
#include <engine/math/Vec2.h>

#include <array>
#include <functional>
#include <memory>
#include <optional>

namespace engine {
class SpriteBatch;
}

namespace game {

// Shown between two scenes: the ranger walking along a trail toward a fire, plus a status
// line. Before a stage it also shows the briefing sheet with a tip.
// Scenes load their assets in OnEnter() on the main thread, so the last frame of this
// screen stays visible while the next one loads. The trail follows a fixed duration,
// not the real loading progress.
class LoadingScene : public engine::Scene {
public:
    using Factory = std::function<std::unique_ptr<engine::Scene>()>;

    // The next scene is created only when the trail is full.
    LoadingScene(Factory next, const char* message, std::optional<config::StageId> stage = {})
        : m_next(std::move(next)), m_message(message), m_stage(stage) {}

    void OnEnter() override;
    void Update(float dt) override;
    void Draw(float alpha) override;

private:
    float FillSeconds() const;
    void DrawBriefing(engine::SpriteBatch& batch, engine::Vec2 center, float s) const;

    Factory m_next;
    const char* m_message;
    std::optional<config::StageId> m_stage; // set = show that stage's briefing
    std::array<std::unique_ptr<engine::Texture>, 4> m_ranger; // walking to the right
    std::unique_ptr<engine::Texture> m_fire;
    engine::Rect m_rangerBounds; // visible part of the first frame
    engine::Rect m_fireSource;
    float m_time = 0.0f;
};

// Loading screen with the briefing, followed by the stage itself.
std::unique_ptr<engine::Scene> LoadStage(config::StageId stage);

} // namespace game
