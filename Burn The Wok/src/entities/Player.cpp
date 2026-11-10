#include "entities/Player.h"

#include "GameConfig.h"
#include "art/PlaceholderArt.h"
#include "world/ForestMap.h"

#include <engine/graphics/SpriteBatch.h>
#include <engine/graphics/Texture.h>
#include <engine/graphics/Image.h>
#include <algorithm>
#include <string>
#include <engine/input/Input.h>
#include <engine/physics/Collision.h>

#include <cmath>

namespace game {

using engine::Key;
using engine::Vec2;

namespace {

// Scale2x refines diagonal pixel-art edges using only existing palette colors.
// This is prepared once on load; nearest filtering keeps the result crisp.
engine::Image BuildDetailedSprite(const engine::Image& source) {
    engine::Image result(source.GetWidth() * 2, source.GetHeight() * 2);
    auto sample = [&](int x, int y) {
        auto color = source.GetPixel(std::clamp(x, 0, source.GetWidth() - 1),
                                     std::clamp(y, 0, source.GetHeight() - 1));
        // Invisible RGB values should not affect silhouette comparisons.
        return color.a == 0 ? engine::Color::Transparent() : color;
    };
    for (int y = 0; y < source.GetHeight(); ++y) {
        for (int x = 0; x < source.GetWidth(); ++x) {
            const auto b = sample(x, y - 1), d = sample(x - 1, y);
            const auto e = sample(x, y), f = sample(x + 1, y);
            const auto h = sample(x, y + 1);
            const bool refine = b != h && d != f;
            result.SetPixel(x * 2, y * 2, refine && d == b ? d : e);
            result.SetPixel(x * 2 + 1, y * 2, refine && b == f ? f : e);
            result.SetPixel(x * 2, y * 2 + 1, refine && d == h ? d : e);
            result.SetPixel(x * 2 + 1, y * 2 + 1, refine && h == f ? f : e);
        }
    }
    return result;
}

} // namespace

void Player::LoadArtwork() {
    // Explicit names preserve the original art team's filenames and frame order.
    const char* files[] = {
        "Front 1.png", "Front 2.png", "Front 3.png", "Front 4.png",
        "Back 11.png", "Back 2.png", "Back 3.png", "Back 4.png",
        "Left 1.png", "Left 2.png", "Left 3.png", "Left 4.png",
        "Right.png", "Right 2.png", "Right 3.png", "Right 4.png"
    };
    float tallest = 1.0f;
    for (int i = 0; i < 16; ++i) {
        const auto image = engine::Image::LoadFromFile(std::string("assets/characters/") + files[i]);
        const auto bounds = image.GetContentBounds(1);
        m_sources[i] = {bounds.x * 2, bounds.y * 2, bounds.w * 2, bounds.h * 2};
        tallest = std::max(tallest, m_sources[i].h);
        m_frames[i] = std::make_unique<engine::Texture>(BuildDetailedSprite(image));
    }
    // One shared scale keeps every pose consistent; transparent canvas padding
    // does not shrink the character. Feet stay anchored to the collision box.
    m_artScale = config::PlayerHeight / tallest;
}

void Player::Update(const engine::Input& input, float dt, const ForestMap& map) {
    m_previousPosition = m_position;

    Vec2 direction{};
    if (input.IsKeyDown(Key::A) || input.IsKeyDown(Key::Left)) direction.x -= 1.0f;
    if (input.IsKeyDown(Key::D) || input.IsKeyDown(Key::Right)) direction.x += 1.0f;
    if (input.IsKeyDown(Key::W) || input.IsKeyDown(Key::Up)) direction.y -= 1.0f;
    if (input.IsKeyDown(Key::S) || input.IsKeyDown(Key::Down)) direction.y += 1.0f;

    m_moving = direction != Vec2{};
    if (m_moving) {
        UpdateFacing(direction);
        // Normalize so diagonal movement is not ~41% faster than straight movement.
        const Vec2 delta = direction.Normalized() * config::PlayerSpeed * dt;
        const engine::Rect hitbox = GetHitbox();
        const engine::MoveResult moved = engine::MoveAndSlide(
            hitbox, delta, static_cast<float>(config::TileSize),
            [&map](int x, int y) { return map.IsSolid(x, y); });
        m_position += moved.box.Position() - hitbox.Position();
    }
    const float distance = (m_position - m_previousPosition).Length();
    m_moving = distance > 0.001f;
    // Distance-driven cadence stops at walls and slows naturally while sliding.
    m_walkTime = m_moving ? std::fmod(m_walkTime + distance / config::PlayerSpeed, 0.4f) : 0.0f;
}

engine::Rect Player::GetHitbox() const {
    return {m_position.x - HitboxWidth * 0.5f, m_position.y + HitboxOffsetY, HitboxWidth, HitboxHeight};
}

void Player::UpdateFacing(Vec2 direction) {
    const bool horizontal = direction.x != 0.0f;
    const bool vertical = direction.y != 0.0f;

    // Moving diagonally: keep the current facing if it is one of the two directions,
    // otherwise the sprite would flicker between them.
    if (horizontal && vertical) {
        const bool stillMatches = (m_facing == Facing::Left && direction.x < 0.0f) ||
                                  (m_facing == Facing::Right && direction.x > 0.0f) ||
                                  (m_facing == Facing::Up && direction.y < 0.0f) ||
                                  (m_facing == Facing::Down && direction.y > 0.0f);
        if (stillMatches) {
            return;
        }
    }

    if (horizontal) {
        m_facing = direction.x < 0.0f ? Facing::Left : Facing::Right;
    } else {
        m_facing = direction.y < 0.0f ? Facing::Up : Facing::Down;
    }
}

Vec2 Player::GetRenderPosition(float alpha) const {
    return engine::Round(engine::Lerp(m_previousPosition, m_position, alpha));
}

void Player::Draw(engine::SpriteBatch& batch, const engine::Texture& atlas, float alpha, float zoom) const {
    SpriteId sprite = SpriteId::PlayerDown;
    switch (m_facing) {
    case Facing::Down:  sprite = SpriteId::PlayerDown;  break;
    case Facing::Up:    sprite = SpriteId::PlayerUp;    break;
    case Facing::Left:  sprite = SpriteId::PlayerLeft;  break;
    case Facing::Right: sprite = SpriteId::PlayerRight; break;
    }

    const Vec2 center = GetRenderPosition(alpha);
    const int frame = static_cast<int>(m_facing) * 4 +
        (m_moving ? std::min(3, static_cast<int>(m_walkTime / 0.1f)) : 0);
    if (m_frames[frame]) {
        const auto source = m_sources[frame];
        // Camera top-left is also screen-pixel snapped, so these edges align
        // with the framebuffer at every supported zoom level.
        const float pixelsPerUnit = std::max(1.0f, zoom);
        const auto snap = [pixelsPerUnit](float value) {
            return std::round(value * pixelsPerUnit) / pixelsPerUnit;
        };
        const float width = snap(source.w * m_artScale);
        const float height = snap(source.h * m_artScale);
        batch.Draw(atlas, {center.x - 6, center.y + 5, 12, 4}, GetSpriteSource(SpriteId::Shadow));
        batch.Draw(*m_frames[frame],
                   {snap(center.x - width * 0.5f), snap(GetRenderFeetY(alpha) - height), width, height}, source);
        return;
    }
    const float size = static_cast<float>(config::TileSize);
    batch.Draw(atlas, {center.x - 5, center.y + 5, 10, 4}, GetSpriteSource(SpriteId::Shadow));
    const float bob = m_moving ? std::round(std::sin(m_walkTime * 16.0f)) : 0.0f;
    batch.Draw(atlas, {center.x - size * 0.5f, center.y - size * 0.5f + bob, size, size},
               GetSpriteSource(sprite));
}

} // namespace game
