#include "art/PlaceholderArt.h"

#include "GameConfig.h"

#include <engine/graphics/Color.h>

namespace game {

namespace {

using engine::Color;
using engine::Image;

constexpr int S = config::TileSize;

// Palette (placeholder, picked to match the "green forest vs. orange fire" art direction).
constexpr Color GrassBase = Color::FromHex(0x65844B);
constexpr Color GrassDark = Color::FromHex(0x567441);
constexpr Color GrassLight = Color::FromHex(0x759453);
constexpr Color Flower = Color::FromHex(0xF4D888);
constexpr Color Bark = Color::FromHex(0x733E39);
constexpr Color WaterBase = Color::FromHex(0x367F87);
constexpr Color WaterLight = Color::FromHex(0x79BBB0);
constexpr Color Skin = Color::FromHex(0xE4A672);
constexpr Color Hair = Color::FromHex(0x3F2832);
constexpr Color Shirt = Color::FromHex(0xD19B53);
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

void PaintGrass(Image& img, int ox, bool flowers, int variant = 0) {
    img.FillRect(ox, 0, S, S, GrassBase);
    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            const unsigned h = Hash(x + variant * 37, y + variant * 19) % 48;
            if (h == 0) {
                img.SetPixel(ox + x, y, GrassDark);
            } else if (h == 1) {
                img.SetPixel(ox + x, y, GrassLight);
                if (y > 0) img.SetPixel(ox + x, y - 1, GrassLight);
            }
        }
    }
    if (flowers) {
        const int spots[][2] = {{3, 4}, {11, 11}};
        for (const auto& p : spots) {
            img.SetPixel(ox + p[0], p[1] + 1, GrassDark);
            img.SetPixel(ox + p[0] - 1, p[1], Color::FromHex(0xECE3B3));
            img.SetPixel(ox + p[0] + 1, p[1], Color::FromHex(0xECE3B3));
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

void PaintWater(Image& img, int ox, int frame = 0) {
    img.FillRect(ox, 0, S, S, WaterBase);
    for (int x = 0; x < S; ++x) {
        if ((x + frame * 3) % 16 < 5) {
            img.SetPixel(ox + x, 4, WaterLight.WithAlpha(255));
        }
        if ((x + 8 - frame * 2 + 16) % 16 < 3) {
            img.SetPixel(ox + x, 12, Color::FromHex(0x559D9B));
        }
    }
}

enum class Pose { Down, Up, Left, Right };

// Forest ranger with a water tank on the back. 16x16, feet at the bottom row.
void PaintPlayer(Image& img, int ox, Pose pose) {
    img.FillRect(ox + 4, 1, 8, 2, Color::FromHex(0xD4B87B)); // ranger hat
    img.FillRect(ox + 4, 2, 8, 1, Color::FromHex(0x9E743F));
    img.FillRect(ox + 3, 3, 10, 1, Color::FromHex(0x62492F));
    img.FillRect(ox + 5, 4, 6, 4, Skin);    // head
    img.FillRect(ox + 4, 8, 8, 5, Shirt);   // body
    img.FillRect(ox + 5, 8, 2, 4, Color::FromHex(0xE2B76C));
    img.FillRect(ox + 10, 8, 2, 5, Color::FromHex(0xA8753E));
    img.FillRect(ox + 4, 12, 8, 1, Boots); // belt
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

void PaintDetails(Image& atlas) {
    int ox = Offset(SpriteId::Dirt);
    atlas.FillRect(ox, 0, S, S, Color::FromHex(0xB29A6B));
    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            const unsigned h = Hash(x + 72, y) % 38;
            if (h < 2) atlas.SetPixel(ox + x, y, Color::FromHex(h ? 0xA58C60 : 0xC0A879));
        }
    }
    ox = Offset(SpriteId::Rock);
    atlas.FillRect(ox + 5, 12, 7, 2, Color::FromHex(0x425641));
    atlas.FillRect(ox + 4, 9, 7, 3, Color::FromHex(0x89927A));
    atlas.FillRect(ox + 6, 7, 5, 3, Color::FromHex(0xA6AE92));
    atlas.FillRect(ox + 5, 9, 3, 1, Color::FromHex(0xC3C9AB));
    ox = Offset(SpriteId::Shrub);
    atlas.FillRect(ox + 3, 12, 10, 2, Color::FromHex(0x49663B));
    atlas.FillRect(ox + 2, 9, 11, 3, Color::FromHex(0x3C6744));
    atlas.FillRect(ox + 4, 7, 7, 4, Color::FromHex(0x507D48));
    atlas.FillRect(ox + 4, 8, 4, 1, Color::FromHex(0x87A658));
    atlas.SetPixel(ox + 5, 10, Color::FromHex(0xCFA27C));
    ox = Offset(SpriteId::Reeds);
    for (int i = 0; i < 4; ++i) {
        const int x = 4 + i * 2;
        atlas.FillRect(ox + x, 7 - i % 3, 1, 8 + i % 3, Color::FromHex(0x3A6242));
        atlas.FillRect(ox + x, 5 - i % 3, 1, 3, Color::FromHex(0xD1B66D));
    }
    ox = Offset(SpriteId::Shadow);
    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            const float dx = (x - 7.5f) / 7.5f;
            const float dy = (y - 7.5f) / 7.5f;
            if (dx * dx + dy * dy < 1.0f) {
                atlas.SetPixel(ox + x, y, Color::FromHex(0x203E2C, 80));
            }
        }
    }
    ox = Offset(SpriteId::Leaf);
    atlas.SetPixel(ox + 7, 7, Color::FromHex(0xD9D799, 165));
    atlas.SetPixel(ox + 8, 8, Color::FromHex(0xC0C88A, 130));
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
    PaintGrass(atlas, Offset(SpriteId::GrassSoft), false, 1);
    PaintGrass(atlas, Offset(SpriteId::GrassDense), false, 2);
    PaintWater(atlas, Offset(SpriteId::WaterRipple), 1);
    PaintWater(atlas, Offset(SpriteId::WaterRippleTwo), 2);
    PaintWater(atlas, Offset(SpriteId::WaterRippleThree), 3);
    PaintDetails(atlas);
    return atlas;
}

engine::Rect GetSpriteSource(SpriteId id) {
    return {static_cast<float>(Offset(id)), 0.0f, static_cast<float>(S), static_cast<float>(S)};
}

} // namespace game
