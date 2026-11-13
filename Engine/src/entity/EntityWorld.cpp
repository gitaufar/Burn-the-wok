#include "engine/entity/EntityWorld.h"

#include <iterator>

namespace engine {

void EntityWorld::Update(float dt) {
    ++m_iterating;
    for (const auto& entity : m_entities) {
        if (!entity->IsAlive()) continue;
        entity->m_previousPosition = entity->m_position;
        entity->Update(dt);
    }
    --m_iterating;

    if (m_iterating == 0) {
        std::erase_if(m_entities, [](const std::unique_ptr<Entity>& e) { return !e->IsAlive(); });
        AddPending();
    }
}

void EntityWorld::Draw(SpriteBatch& batch, float alpha) const {
    for (const auto& entity : m_entities) {
        if (entity->IsAlive()) entity->Draw(batch, alpha);
    }
}

void EntityWorld::AddPending() {
    std::move(m_pending.begin(), m_pending.end(), std::back_inserter(m_entities));
    m_pending.clear();
}

void EntityWorld::Clear() {
    m_entities.clear();
    m_pending.clear();
}

} // namespace engine
