#pragma once

#include "engine/math/Mat4.h"
#include "engine/math/Rect.h"
#include "engine/math/Vec2.h"

namespace engine {

// 2D camera. The position is the world point shown at the CENTER of the viewport;
// zoom is how many screen pixels one world unit covers (zoom 3 -> a 16 px tile is 48 px on screen).
// World and screen are both y-down.
//
// With pixel snapping on (default), the visible area's top-left corner is rounded to
// whole screen pixels so pixel art does not shimmer while the camera moves.
// Use integer zoom levels for perfectly square pixels.
class Camera2D {
public:
    void SetViewportSize(Vec2 sizeInPixels) { m_viewport = sizeInPixels; }
    Vec2 GetViewportSize() const { return m_viewport; }

    void SetPosition(Vec2 worldCenter) { m_position = worldCenter; }
    Vec2 GetPosition() const { return m_position; }

    void SetZoom(float zoom);
    float GetZoom() const { return m_zoom; }

    void SetPixelSnap(bool enabled) { m_pixelSnap = enabled; }

    // World-space rectangle currently visible. Use it to skip drawing off-screen tiles.
    Rect GetVisibleArea() const;
    Mat4 GetViewProjection() const;

    Vec2 ScreenToWorld(Vec2 screen) const;
    Vec2 WorldToScreen(Vec2 world) const;

private:
    Vec2 GetTopLeft() const;

    Vec2 m_position{};
    Vec2 m_viewport{1.0f, 1.0f};
    float m_zoom = 1.0f;
    bool m_pixelSnap = true;
};

} // namespace engine
