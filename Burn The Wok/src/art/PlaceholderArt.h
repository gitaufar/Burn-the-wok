#pragma once

#include <engine/graphics/Image.h>
#include <engine/math/Rect.h>

namespace game {

// Sprites in the placeholder atlas, left to right, one tile each.
enum class SpriteId : int {
    Grass,
    GrassFlowers,
    Tree,
    Water,
    PlayerDown,
    PlayerUp,
    PlayerLeft,
    PlayerRight,
    GrassSoft,
    GrassDense,
    Dirt,
    Rock,
    Shrub,
    Reeds,
    Shadow,
    Leaf,
    WaterRipple,
    WaterRippleTwo,
    WaterRippleThree,
    FireA, // three flame frames, played in a loop
    FireB,
    FireC,
    Glow,  // white radial gradient; tint it (e.g. orange light under a fire)
    Count
};

// Builds a small sprite atlas in code, so Stage 1 runs without any image files
// (and without an image decoder, which is not approved yet). Replace with real
// pixel art once assets exist; only this file needs to change.
engine::Image BuildPlaceholderAtlas();

// Oil palm ("sawit") that replaces a tree burnt for too long. Its own image (not an
// atlas cell) because it is taller than a tile; drawn 1:1 with its roots on the tile.
engine::Image BuildSawitImage();

// Source rectangle of a sprite inside the atlas, in texture pixels.
engine::Rect GetSpriteSource(SpriteId id);

} // namespace game
