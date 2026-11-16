#pragma once

#include "GameConfig.h"

#include <engine/entity/Entity.h>
#include <engine/graphics/Texture.h>
#include <engine/math/Rect.h>

#include <array>
#include <functional>
#include <memory>
#include <vector>

namespace engine { class EntityWorld; }

namespace game {

class ForestMap;
class FireSystem;

class StageActor final : public engine::Entity {
public:
    using Kind = config::ActorKind;
    enum class Rescue { Waiting, Panicked, Fleeing, Saved, Burned };
    struct Cell {
        int x;
        int y;
        bool operator==(const Cell&) const = default;
    };

    StageActor(Kind kind, engine::Vec2 position);
    void LoadArtwork();
    void Draw(engine::SpriteBatch& batch, float alpha) const override;
    void Draw(engine::SpriteBatch& batch, float alpha, float zoom) const;
    void Step(float dt, ForestMap& map, engine::EntityWorld& world, FireSystem& fires,
              engine::Vec2 ranger, const engine::Rect& rangerFeet);
    bool IsAnimal() const { return m_kind <= Kind::Owl; }
    bool BlocksRanger() const { return m_kind == Kind::Wowo || m_kind == Kind::Tedduy; }
    bool IsPlanting() const { return m_planting; }
    Rescue GetRescue() const { return m_rescue; }
    engine::Rect GetFeet() const;
    static Cell ToCell(engine::Vec2 position);
    static engine::Vec2 CellCenter(Cell cell);
    static std::vector<Cell> FindRoute(const ForestMap& map, Cell start,
                                      const std::function<bool(Cell)>& goal);

private:
    bool MoveToward(engine::Vec2 target, float speed, float dt, const ForestMap& map);
    void FollowRoute(float speed, float dt, const ForestMap& map);
    void StepAnimal(float dt, const ForestMap& map, const engine::EntityWorld& world, engine::Vec2 ranger);
    void StepSaboteur(float dt, ForestMap& map, engine::EntityWorld& world, FireSystem& fires,
                      const engine::Rect& rangerFeet);
    void StepBoss(float dt, const ForestMap& map, engine::Vec2 ranger);

    Kind m_kind;
    Rescue m_rescue = Rescue::Waiting;
    std::array<std::unique_ptr<engine::Texture>, 16> m_frames;
    std::array<engine::Rect, 16> m_sources{};
    float m_artScale = 1.0f;
    float m_walkTime = 0.0f;
    int m_facing = 0;
    std::vector<Cell> m_route;
    int m_routeRevision = -1;
    float m_exposure = 0.0f;
    float m_panicTime = 0.0f;
    float m_actionTimer = 0.0f;
    int m_ignitionsLeft = 2;
    bool m_planting = false;
};

} // namespace game
