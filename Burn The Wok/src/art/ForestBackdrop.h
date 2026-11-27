#pragma once

#include <engine/graphics/SpriteBatch.h>
#include <engine/math/Vec2.h>

#include <cstdint>

namespace game::art {

inline void DrawForestTree(engine::SpriteBatch& batch, float x, float ground, float height,
                           float s, engine::Color color) {
    const float width = height * 0.65f;
    batch.DrawRect({x - width * 0.08f, ground - height, width * 0.16f, height}, color);
    batch.DrawRect({x - width * 0.50f, ground - height * 0.88f, width, height * 0.34f}, color);
    batch.DrawRect({x - width * 0.38f, ground - height * 1.02f, width * 0.76f, height * 0.28f}, color);
    batch.DrawRect({x - width * 0.22f, ground - height * 1.10f, width * 0.44f, height * 0.20f}, color);
    batch.DrawRect({x - width * 0.58f, ground - height * 0.69f, width * 1.16f, 18.0f * s}, color);
}

// Night-green sky shared by the menu-style screens (screen space, origin top-left).
inline void DrawForestSky(engine::SpriteBatch& batch, engine::Vec2 size) {
    for (int i = 0; i < 48; ++i) {
        const float t = static_cast<float>(i) / 47.0f;
        batch.DrawRect({0.0f, size.y * i / 48.0f, size.x, size.y / 48.0f + 1.0f},
            {static_cast<std::uint8_t>(7 + 7 * t), static_cast<std::uint8_t>(20 + 16 * t),
             static_cast<std::uint8_t>(17 + 9 * t), 255});
    }
}

// Two layers of tree silhouettes standing on `ground`, plus the dark soil below them.
inline void DrawForestSilhouettes(engine::SpriteBatch& batch, engine::Vec2 size, float ground, float s) {
    for (int layer = 0; layer < 2; ++layer) {
        const float spacing = (layer == 0 ? 105.0f : 155.0f) * s;
        const auto color = engine::Color::FromHex(layer == 0 ? 0x102F27 : 0x091D18);
        for (int i = 0; i * spacing < size.x + spacing; ++i) {
            const float x = i * spacing + ((layer == 0) ? 0.0f : 37.0f * s);
            const float height = (180.0f + static_cast<float>((i * 71 + layer * 43) % 120)) * s;
            DrawForestTree(batch, x, ground + layer * 34.0f * s, height, s, color);
        }
    }
    const float soil = ground + 34.0f * s;
    batch.DrawRect({0.0f, soil, size.x, size.y > soil ? size.y - soil : 0.0f},
                   engine::Color::FromHex(0x091D18));
}

} // namespace game::art
