#pragma once

#include "engine/math/Vec2.h"

namespace engine {

class SpriteBatch;

// Base class for every dynamic object in a world (fires, particles, animals, ...).
// Deliberately small: a position, an alive flag and Update/Draw hooks. Interactions
// between entities (e.g. water hitting fire) belong in game systems, not in here,
// so an entity never needs to know about the others.
class Entity {
public:
    explicit Entity(Vec2 position = {}) : m_position(position), m_previousPosition(position) {}
    virtual ~Entity() = default;

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    // dt = fixed update step in seconds.
    virtual void Update(float /*dt*/) {}
    // alpha = interpolation factor passed to Scene::Draw.
    virtual void Draw(SpriteBatch& /*batch*/, float /*alpha*/) const {}

    // Marks the entity for removal. EntityWorld deletes it at the end of the current
    // update, so references stay valid while other entities are still running.
    void Destroy() { m_alive = false; }
    bool IsAlive() const { return m_alive; }

    Vec2 GetPosition() const { return m_position; }
    // Teleport: also resets the interpolation start so the entity does not "slide" there.
    void SetPosition(Vec2 position) { m_position = m_previousPosition = position; }
    // Position between the previous and the current update.
    Vec2 GetRenderPosition(float alpha) const { return Lerp(m_previousPosition, m_position, alpha); }

protected:
    Vec2 m_position;

private:
    friend class EntityWorld;
    Vec2 m_previousPosition;
    bool m_alive = true;
};

} // namespace engine
