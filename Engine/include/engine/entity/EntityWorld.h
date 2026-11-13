#pragma once

#include "engine/entity/Entity.h"

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace engine {

// Owns a list of entities and runs them. Not a full ECS on purpose: a small 2D game
// only needs "spawn, update all, draw all, find all of type T, remove the dead ones".
//
//   auto& fire = world.Spawn<Fire>(position);   // returns the new entity
//   world.Update(dt);                            // Update() on every entity, then cleanup
//   world.ForEach<Fire>([](Fire& f) { ... });    // only live entities of that type
//
// Spawning while the world is being iterated (inside Update() or a ForEach callback)
// is safe: the new entity joins when that iteration finishes, so the list is never
// modified mid-loop. Destroyed entities are skipped at once and deleted in Update().
class EntityWorld {
public:
    EntityWorld() = default;
    EntityWorld(const EntityWorld&) = delete;
    EntityWorld& operator=(const EntityWorld&) = delete;

    template <class T, class... Args>
    T& Spawn(Args&&... args) {
        static_assert(std::is_base_of_v<Entity, T>, "T must derive from engine::Entity");
        auto entity = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *entity;
        (m_iterating > 0 ? m_pending : m_entities).push_back(std::move(entity));
        return ref;
    }

    // Saves each entity's previous position (for interpolation), calls Update(),
    // removes destroyed entities, then adds entities spawned during the update.
    void Update(float dt);
    // Draws live entities in spawn order (later spawns on top).
    void Draw(SpriteBatch& batch, float alpha) const;

    // Calls fn(T&) for every live entity whose type is T (or derives from T).
    template <class T, class Fn>
    void ForEach(Fn&& fn) {
        ++m_iterating;
        for (const auto& entity : m_entities) {
            if (!entity->IsAlive()) continue;
            if (T* typed = dynamic_cast<T*>(entity.get())) fn(*typed);
        }
        if (--m_iterating == 0) AddPending();
    }
    template <class T, class Fn>
    void ForEach(Fn&& fn) const {
        for (const auto& entity : m_entities) {
            if (!entity->IsAlive()) continue;
            if (const T* typed = dynamic_cast<const T*>(entity.get())) fn(*typed);
        }
    }

    template <class T>
    int Count() const {
        int count = 0;
        ForEach<T>([&count](const T&) { ++count; });
        return count;
    }

    // All entities, including ones spawned but not yet added.
    std::size_t GetSize() const { return m_entities.size() + m_pending.size(); }
    void Clear();

private:
    void AddPending();

    std::vector<std::unique_ptr<Entity>> m_entities;
    std::vector<std::unique_ptr<Entity>> m_pending; // spawned while iterating
    int m_iterating = 0; // nesting depth of Update()/ForEach() loops over m_entities
};

} // namespace engine
