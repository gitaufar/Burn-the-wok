#pragma once
#include <engine/graphics/SpriteBatch.h>

namespace game::art {
enum class FieldIcon { Tank, Nozzle, Pump, Boot, Crew, Megaphone, Leaf, Lock, Flag, Star };

inline void DrawFieldIcon(engine::SpriteBatch& batch, FieldIcon icon, float x, float y, float scale,
                          engine::Color color) {
    constexpr unsigned char glyphs[][8] = {
        {24,28,60,102,126,126,126,60}, {3,7,126,252,120,48,48,0},
        {56,124,108,63,29,24,60,126}, {96,112,112,120,126,127,127,0},
        {24,60,126,36,24,126,90,126}, {3,15,127,127,126,56,48,0},
        {7,15,27,55,110,124,56,96}, {60,102,66,255,231,231,255,126},
        {254,254,252,248,128,128,128,192}, {24,24,255,126,60,126,102,0}
    };
    const auto& rows = glyphs[static_cast<int>(icon)];
    for (int row = 0; row < 8; ++row)
        for (int col = 0; col < 8; ++col)
            if (rows[row] & (128 >> col))
                batch.DrawRect({x+col*scale, y+row*scale, scale, scale}, color);
}
}
