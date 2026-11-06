#pragma once

#include <engine/graphics/SpriteBatch.h>
#include <string_view>

namespace game::art {

// Small built-in font for in-game labels; no external font asset is needed.
inline void DrawPixelText(engine::SpriteBatch& batch, std::string_view text,
                          float x, float y, float scale, engine::Color color) {
    static constexpr unsigned char letters[36][7] = {
        {14,17,17,31,17,17,17}, {30,17,17,30,17,17,30},
        {14,17,16,16,16,17,14}, {30,17,17,17,17,17,30},
        {31,16,16,30,16,16,31}, {31,16,16,30,16,16,16},
        {14,17,16,23,17,17,14}, {17,17,17,31,17,17,17},
        {31,4,4,4,4,4,31}, {7,2,2,2,18,18,12},
        {17,18,20,24,20,18,17}, {16,16,16,16,16,16,31},
        {17,27,21,21,17,17,17}, {17,25,21,19,17,17,17},
        {14,17,17,17,17,17,14}, {30,17,17,30,16,16,16},
        {14,17,17,17,21,18,13}, {30,17,17,30,20,18,17},
        {15,16,16,14,1,1,30}, {31,4,4,4,4,4,4},
        {17,17,17,17,17,17,14}, {17,17,17,17,17,10,4},
        {17,17,17,21,21,21,10}, {17,17,10,4,10,17,17},
        {17,17,10,4,4,4,4}, {31,1,2,4,8,16,31},
        {14,17,19,21,25,17,14}, {4,12,4,4,4,4,14},
        {14,17,1,2,4,8,31}, {30,1,1,14,1,1,30},
        {2,6,10,18,31,2,2}, {31,16,16,30,1,1,30},
        {14,16,16,30,17,17,14}, {31,1,2,4,8,8,8},
        {14,17,17,14,17,17,14}, {14,17,17,15,1,1,14}
    };
    for (char ch : text) {
        const int index = ch >= 'A' && ch <= 'Z' ? ch - 'A' :
                          ch >= '0' && ch <= '9' ? ch - '0' + 26 : -1;
        if (index >= 0) {
            for (int row = 0; row < 7; ++row) {
                for (int col = 0; col < 5; ++col) {
                    if (letters[index][row] & (1 << (4 - col))) {
                        batch.DrawRect({x + col * scale, y + row * scale, scale, scale}, color);
                    }
                }
            }
        } else if (ch == '-') {
            batch.DrawRect({x, y + 3 * scale, 4 * scale, scale}, color);
        } else if (ch == ':') {
            batch.DrawRect({x + scale, y + 2 * scale, scale, scale}, color);
            batch.DrawRect({x + scale, y + 5 * scale, scale, scale}, color);
        }
        x += 6 * scale;
    }
}

} // namespace game::art
