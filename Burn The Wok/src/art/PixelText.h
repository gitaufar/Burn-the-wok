#pragma once

#include <engine/graphics/SpriteBatch.h>
#include <string_view>

namespace game::art {

// Extra punctuation for longer texts (intro narration). 5x7 rows, bit 4 = leftmost column.
inline const unsigned char* PunctuationRows(char ch) {
    static constexpr unsigned char comma[7] = {0, 0, 0, 0, 12, 12, 8};
    static constexpr unsigned char question[7] = {14, 17, 1, 2, 4, 0, 4};
    static constexpr unsigned char exclaim[7] = {4, 4, 4, 4, 4, 0, 4};
    static constexpr unsigned char semicolon[7] = {0, 12, 12, 0, 12, 4, 8};
    static constexpr unsigned char apostrophe[7] = {4, 4, 8, 0, 0, 0, 0};
    static constexpr unsigned char quote[7] = {10, 10, 0, 0, 0, 0, 0};
    static constexpr unsigned char open[7] = {2, 4, 8, 8, 8, 4, 2};
    static constexpr unsigned char close[7] = {8, 4, 2, 2, 2, 4, 8};
    switch (ch) {
    case ',': return comma;
    case '?': return question;
    case '!': return exclaim;
    case ';': return semicolon;
    case '\'': return apostrophe;
    case '"': return quote;
    case '(': return open;
    case ')': return close;
    default: return nullptr;
    }
}

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
    for (char raw : text) {
        const char ch = raw >= 'a' && raw <= 'z' ? static_cast<char>(raw - 'a' + 'A') : raw; // font is upper-case only
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
        } else if (ch == '%' || ch == '/' || ch == '.') {
            static constexpr unsigned char percent[7] = {24, 25, 2, 4, 8, 19, 3};
            static constexpr unsigned char slash[7] = {1, 1, 2, 4, 8, 16, 16};
            static constexpr unsigned char dot[7] = {0, 0, 0, 0, 0, 12, 12};
            const unsigned char* rows = ch == '%' ? percent : ch == '/' ? slash : dot;
            for (int row = 0; row < 7; ++row) {
                for (int col = 0; col < 5; ++col) {
                    if (rows[row] & (1 << (4 - col))) {
                        batch.DrawRect({x + col * scale, y + row * scale, scale, scale}, color);
                    }
                }
            }
        } else if (ch == '-') {
            batch.DrawRect({x, y + 3 * scale, 4 * scale, scale}, color);
        } else if (ch == ':') {
            batch.DrawRect({x + scale, y + 2 * scale, scale, scale}, color);
            batch.DrawRect({x + scale, y + 5 * scale, scale, scale}, color);
        } else if (const unsigned char* rows = PunctuationRows(ch)) {
            for (int row = 0; row < 7; ++row) {
                for (int col = 0; col < 5; ++col) {
                    if (rows[row] & (1 << (4 - col))) {
                        batch.DrawRect({x + col * scale, y + row * scale, scale, scale}, color);
                    }
                }
            }
        }
        x += 6 * scale;
    }
}

} // namespace game::art
