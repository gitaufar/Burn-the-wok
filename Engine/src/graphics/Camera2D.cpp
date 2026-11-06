#include "engine/graphics/Camera2D.h"

#include <cmath>

namespace engine {

void Camera2D::SetZoom(float zoom) {
    m_zoom = zoom > 0.0f ? zoom : 1.0f;
}

Vec2 Camera2D::GetTopLeft() const {
    Vec2 topLeft = m_position - m_viewport / (2.0f * m_zoom);
    if (m_pixelSnap) {
        // Round to a whole screen pixel (1 screen pixel = 1/zoom world units).
        topLeft.x = std::round(topLeft.x * m_zoom) / m_zoom;
        topLeft.y = std::round(topLeft.y * m_zoom) / m_zoom;
    }
    return topLeft;
}

Rect Camera2D::GetVisibleArea() const {
    const Vec2 topLeft = GetTopLeft();
    return {topLeft.x, topLeft.y, m_viewport.x / m_zoom, m_viewport.y / m_zoom};
}

Mat4 Camera2D::GetViewProjection() const {
    const Rect area = GetVisibleArea();
    // bottom > top: world y grows downward, like the screen.
    return Mat4::Ortho(area.Left(), area.Right(), area.Bottom(), area.Top());
}

Vec2 Camera2D::ScreenToWorld(Vec2 screen) const {
    return GetTopLeft() + screen / m_zoom;
}

Vec2 Camera2D::WorldToScreen(Vec2 world) const {
    return (world - GetTopLeft()) * m_zoom;
}

} // namespace engine
