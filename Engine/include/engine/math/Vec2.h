#pragma once

#include <cmath>

namespace engine {

// 2D vector used for positions, sizes and directions.
// Plain aggregate on purpose: Vec2{1.0f, 2.0f} just works and copies are cheap.
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vec2 operator+(Vec2 o) const { return {x + o.x, y + o.y}; }
    constexpr Vec2 operator-(Vec2 o) const { return {x - o.x, y - o.y}; }
    constexpr Vec2 operator*(float s) const { return {x * s, y * s}; }
    constexpr Vec2 operator/(float s) const { return {x / s, y / s}; }
    constexpr Vec2 operator-() const { return {-x, -y}; }

    constexpr Vec2& operator+=(Vec2 o) { x += o.x; y += o.y; return *this; }
    constexpr Vec2& operator-=(Vec2 o) { x -= o.x; y -= o.y; return *this; }
    constexpr Vec2& operator*=(float s) { x *= s; y *= s; return *this; }

    constexpr bool operator==(const Vec2&) const = default;

    constexpr float LengthSquared() const { return x * x + y * y; }
    float Length() const { return std::sqrt(LengthSquared()); }

    // Returns a zero vector instead of NaN when the length is zero.
    Vec2 Normalized() const {
        const float len = Length();
        return len > 0.0f ? *this / len : Vec2{};
    }
};

constexpr Vec2 operator*(float s, Vec2 v) { return v * s; }

constexpr Vec2 Lerp(Vec2 a, Vec2 b, float t) { return a + (b - a) * t; }

inline Vec2 Round(Vec2 v) { return {std::round(v.x), std::round(v.y)}; }

} // namespace engine
