#pragma once

#include "systems/Sprayer.h"
#include "systems/WaterTank.h"
#include "world/Navigation.h"
#include <engine/entity/Entity.h>

namespace engine { class EntityWorld; }

namespace game {
class Volunteer final : public engine::Entity {
public:
    Volunteer(engine::Vec2 start, unsigned seed, float duty);
    void Step(float dt, const ForestMap& map, engine::EntityWorld& world);
    void Draw(engine::SpriteBatch& batch, float alpha) const override;
    static bool IsExit(const ForestMap& map, navigation::Cell cell);

private:
    void Walk(float dt, float speedScale);
    std::vector<navigation::Cell> m_route;
    WaterTank m_tank{40};
    Sprayer m_sprayer;
    float m_duty;
    bool m_temporary;
    bool m_refill = false;
    float m_replan = 0;
    int m_mapRevision = -1;
    float m_stride = 0;
    engine::Vec2 m_facing{0, 1};
};
}
