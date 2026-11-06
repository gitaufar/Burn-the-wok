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
    Count
};

// Builds a small sprite atlas in code, so Stage 1 runs without any image files
// (and without an image decoder, which is not approved yet). Replace with real
// pixel art once assets exist; only this file needs to change.
engine::Image BuildPlaceholderAtlas();

// Source rectangle of a sprite inside the atlas, in texture pixels.
engine::Rect GetSpriteSource(SpriteId id);

} // namespace game
