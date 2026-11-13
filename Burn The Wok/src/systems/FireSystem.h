#pragma once

#include <random>

namespace engine {
class EntityWorld;
class Texture;
} // namespace engine

namespace game {

class ForestMap;

// Rules that involve fires and other entities or the map: where fires start, how they
// spread, what a long fire does to the land, and what happens when water reaches them.
// The Fire entity itself only knows its own state.
class FireSystem {
public:
    // atlas is handed to every Fire it creates and must outlive them.
    FireSystem(const engine::Texture& atlas, unsigned seed) : m_atlas(&atlas), m_rng(seed) {}

    // What happened during one Update().
    struct StepResult {
        int spread = 0;        // new fires lit by sparks
        int turnedToSawit = 0; // trees lost to the plantation
    };

    // Sets `count` random trees on fire. Only trees the player can reach (with at least
    // one walkable neighbour) and not on the map border are chosen. Returns how many
    // were actually lit (fewer if the map has fewer candidates).
    int IgniteRandomTrees(engine::EntityWorld& world, const ForestMap& map, int count);

    // Lights tile (x, y). Fails (returns false) if it is not a burnable tree
    // (ForestMap::IsBurnable) or already burning.
    bool Ignite(engine::EntityWorld& world, const ForestMap& map, int tileX, int tileY);
    bool IsBurning(const engine::EntityWorld& world, int tileX, int tileY) const;

    // Water drops that touch a fire are used up and lower its intensity. Fires that reach
    // zero are removed and leave a puff of smoke. Returns the number put out this step.
    int ResolveWaterHits(engine::EntityWorld& world);

    // Call once per update, after the fires' own Update():
    //  - burning trees get scorched; a tree burnt for BurnToSawitSeconds becomes Sawit
    //    and its fire goes out (nothing left to burn);
    //  - a fire that is ready to spread lights a random burnable tree within
    //    SpreadRadiusTiles (if there is one).
    StepResult Update(engine::EntityWorld& world, ForestMap& map, float dt);

private:
    const engine::Texture* m_atlas;
    std::mt19937 m_rng;
};

} // namespace game
