#include "art/PlaceholderArt.h"

#include "GameConfig.h"

#include <engine/graphics/Color.h>

namespace game {

namespace {

using engine::Color;
using engine::Image;

constexpr int S = config::TileSize;

// Palette (placeholder, picked to match the "green forest vs. orange fire" art direction).
constexpr Color GrassBase = Color::FromHex(0x3E8948);
constexpr Color GrassDark = Color::FromHex(0x265C42);
constexpr Color GrassLight = Color::FromHex(0x63C74D);
constexpr Color Flower = Color::FromHex(0xFEE761);
constexpr Color Bark = Color::FromHex(0x733E39);
constexpr Color WaterBase = Color::FromHex(0x0099DB);
constexpr Color WaterLight = Color::FromHex(0x2CE8F5);
constexpr Color Skin = Color::FromHex(0xE4A672);
constexpr Color Hair = Color::FromHex(0x3F2832);
constexpr Color Shirt = Color::FromHex(0x5D7A3A);
constexpr Color Pants = Color::FromHex(0x3A4466);
constexpr Color Boots = Color::FromHex(0x262B44);
constexpr Color Metal = Color::FromHex(0x8B9BB4);

// Tiny deterministic hash: the same (x, y) always gives the same "random" value,
// so the grass texture looks noisy but never changes between runs.
unsigned Hash(int x, int y) {
    unsigned h = static_cast<unsigned>(x) * 374761393u + static_cast<unsigned>(y) * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

void PaintGrass(Image& img, int ox, bool flowers) {
    img.FillRect(ox, 0, S, S, GrassBase);
    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            const unsigned h = Hash(x, y) % 16;
            if (h == 0) {
                img.SetPixel(ox + x, y, GrassDark);
            } else if (h == 1) {
                img.SetPixel(ox + x, y, GrassLight);
            }
        }
    }
    if (flowers) {
        const int spots[][2] = {{3, 4}, {11, 2}, {7, 10}, {13, 12}, {2, 13}};
        for (const auto& p : spots) {
            img.SetPixel(ox + p[0], p[1], Flower);
        }
    }
}

void PaintTree(Image& img, int ox) {
    img.FillRect(ox + 7, 10, 2, 6, Bark); // trunk
    // Round canopy: every pixel within radius 6 of the canopy center.
    for (int y = 0; y < 13; ++y) {
        for (int x = 0; x < S; ++x) {
            const float dx = x - 7.5f;
            const float dy = y - 6.0f;
            const float d2 = dx * dx + dy * dy;
            if (d2 <= 36.0f) {
                // Lighter pixels toward the top-left fake a light source.
                const bool lit = d2 <= 16.0f && dx + dy < -1.0f;
                img.SetPixel(ox + x, y, lit ? GrassBase : GrassDark);
            }
        }
    }
}

void PaintWater(Image& img, int ox) {
    img.FillRect(ox, 0, S, S, WaterBase);
    for (int x = 0; x < S; ++x) {
        if (x % 6 < 3) {
            img.SetPixel(ox + x, 4, WaterLight);
        }
        if ((x + 3) % 6 < 3) {
            img.SetPixel(ox + x, 11, WaterLight);
        }
    }
}

enum class Pose { Down, Up, Left, Right };

// Forest ranger with a water tank on the back. 16x16, feet at the bottom row.
void PaintPlayer(Image& img, int ox, Pose pose) {
    img.FillRect(ox + 4, 1, 8, 2, Bark);    // hat
    img.FillRect(ox + 3, 3, 10, 1, Bark);   // hat brim
    img.FillRect(ox + 5, 4, 6, 4, Skin);    // head
    img.FillRect(ox + 4, 8, 8, 5, Shirt);   // body
    img.FillRect(ox + 5, 13, 2, 2, Pants);  // legs
    img.FillRect(ox + 9, 13, 2, 2, Pants);
    img.FillRect(ox + 5, 15, 2, 1, Boots);
    img.FillRect(ox + 9, 15, 2, 1, Boots);

    switch (pose) {
    case Pose::Down:
        img.SetPixel(ox + 6, 5, Boots); // eyes
        img.SetPixel(ox + 9, 5, Boots);
        break;
    case Pose::Up:
        img.FillRect(ox + 5, 4, 6, 3, Hair);      // back of the head
        img.FillRect(ox + 5, 8, 6, 4, WaterBase); // water tank
        break;
    case Pose::Left:
        img.SetPixel(ox + 5, 5, Boots);
        img.FillRect(ox + 11, 8, 2, 4, WaterBase);
        img.SetPixel(ox + 3, 10, Metal); // sprayer nozzle
        break;
    case Pose::Right:
        img.SetPixel(ox + 10, 5, Boots);
        img.FillRect(ox + 3, 8, 2, 4, WaterBase);
        img.SetPixel(ox + 12, 10, Metal);
        break;
    }
}

int Offset(SpriteId id) {
    return static_cast<int>(id) * S;
}

} // namespace

engine::Image BuildPlaceholderAtlas() {
    Image atlas(S * static_cast<int>(SpriteId::Count), S, Color::Transparent());
    PaintGrass(atlas, Offset(SpriteId::Grass), false);
    PaintGrass(atlas, Offset(SpriteId::GrassFlowers), true);
    PaintTree(atlas, Offset(SpriteId::Tree));
    PaintWater(atlas, Offset(SpriteId::Water));
    PaintPlayer(atlas, Offset(SpriteId::PlayerDown), Pose::Down);
    PaintPlayer(atlas, Offset(SpriteId::PlayerUp), Pose::Up);
    PaintPlayer(atlas, Offset(SpriteId::PlayerLeft), Pose::Left);
    PaintPlayer(atlas, Offset(SpriteId::PlayerRight), Pose::Right);
    return atlas;
}

engine::Rect GetSpriteSource(SpriteId id) {
    return {static_cast<float>(Offset(id)), 0.0f, static_cast<float>(S), static_cast<float>(S)};
}

} // namespace game
