#pragma once

namespace engine {

// 4x4 matrix stored column-major (m[column * 4 + row]), which is the layout
// OpenGL expects, so it can be uploaded with glUniformMatrix4fv(..., GL_FALSE, ...).
// A 2D engine only needs an orthographic projection, so that is all we provide.
struct Mat4 {
    float m[16] = {};

    static constexpr Mat4 Identity() {
        Mat4 r;
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }

    // Maps the box [left, right] x [bottom, top] x [-1, 1] onto clip space.
    // Passing bottom > top gives a y-down coordinate system.
    static constexpr Mat4 Ortho(float left, float right, float bottom, float top) {
        Mat4 r;
        r.m[0] = 2.0f / (right - left);
        r.m[5] = 2.0f / (top - bottom);
        r.m[10] = -1.0f;
        r.m[12] = -(right + left) / (right - left);
        r.m[13] = -(top + bottom) / (top - bottom);
        r.m[15] = 1.0f;
        return r;
    }

    const float* Data() const { return m; }
};

} // namespace engine
