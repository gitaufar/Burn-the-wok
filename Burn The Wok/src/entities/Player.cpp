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

// The ranger frames (assets/characters/ranger, built by tools/assets/compose_ranger.py)
// all share one layout, in image pixels: the head is centered on the canvas and the
// lowest body pixel sits on row ArtFeetY. Nozzle = tip of the spray gun in each facing,
// relative to (canvas center, ArtFeetY); the script prints these numbers.
constexpr float ArtFeetY = 76.0f;
constexpr Vec2 NozzleDown{12.0f, -10.0f};  // gun held at the side, pointing at the ground
constexpr Vec2 NozzleUp{-12.0f, -34.0f};
constexpr Vec2 NozzleLeft{-21.0f, -18.0f};
constexpr Vec2 NozzleRight{21.0f, -19.0f};

} // namespace

void Player::LoadArtwork() {
    // Two sets (walking, then "spray_" = holding the gun); in each, rows follow the
    // Facing enum (Down, Up, Left, Right) with 4 walk frames.
    const char* directions[] = {"front", "back", "left", "right"};
    float tallest = 1.0f;
    for (int i = 0; i < FramesPerSet * 2; ++i) {
        const int inSet = i % FramesPerSet;
        const auto image = engine::Image::LoadFromFile(
            std::string("assets/characters/ranger/") + (i < FramesPerSet ? "" : "spray_") +
            directions[inSet / 4] + "_" + std::to_string(inSet % 4 + 1) + ".png");
        // Whole canvas as source: the frames are pre-aligned, so one fixed anchor
        // works for all of them and the gun/tank never shift the body sideways.
        m_sources[i] = {0.0f, 0.0f, image.GetWidth() * 2.0f, image.GetHeight() * 2.0f};
        tallest = std::max(tallest, image.GetContentBounds(128).h * 2.0f);
        m_frames[i] = std::make_unique<engine::Texture>(BuildDetailedSprite(image));
    }
    // One shared scale keeps every pose consistent: the tallest pose is PlayerHeight.
    m_artScale = config::PlayerHeight / tallest;
}

void Player::Update(const engine::Input& input, float dt, const ForestMap& map) {
    m_previousPosition = m_position;
    m_aiming = false; // set again by Aim() if the trigger is still held

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
    // "Touching" the pond = the hitbox grown by a few pixels overlaps a water tile.
    engine::Rect reach = GetHitbox();
    reach.x -= config::RefillReach;
    reach.y -= config::RefillReach;
    reach.w += config::RefillReach * 2.0f;
    reach.h += config::RefillReach * 2.0f;
    m_refilling = !m_tank.IsFull() && map.HasTileInArea(reach, TileType::Water);
    if (m_refilling) {
        m_tank.Refill(config::RefillPerSecond * dt);
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

Facing Player::FacingFor(Vec2 direction) {
    if (std::abs(direction.x) > std::abs(direction.y)) {
        return direction.x < 0.0f ? Facing::Left : Facing::Right;
    }
    return direction.y < 0.0f ? Facing::Up : Facing::Down;
}

void Player::Aim(Vec2 direction) {
    if (direction == Vec2{}) return;
    // Pick the dominant axis; UpdateFacing would keep the old facing on diagonals.
    m_facing = FacingFor(direction);
    m_aiming = true;
}

Vec2 Player::GetNozzlePosition(Vec2 aim) const {
    Vec2 nozzle = NozzleDown;
    switch (aim == Vec2{} ? m_facing : FacingFor(aim)) {
    case Facing::Down:  nozzle = NozzleDown;  break;
    case Facing::Up:    nozzle = NozzleUp;    break;
    case Facing::Left:  nozzle = NozzleLeft;  break;
    case Facing::Right: nozzle = NozzleRight; break;
    }
    // Image pixels -> world: frames are doubled by Scale2x, then scaled by m_artScale.
    const Vec2 feet{m_position.x, m_position.y + HitboxOffsetY + HitboxHeight};
    return feet + nozzle * (2.0f * m_artScale);
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
    const int frame = (m_aiming ? FramesPerSet : 0) + static_cast<int>(m_facing) * 4 +
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
        // Row ArtFeetY of the image (doubled by Scale2x) rests on the feet line.
        const float feetOffset = ArtFeetY * 2.0f * m_artScale;
        batch.Draw(*m_frames[frame],
                   {snap(center.x - width * 0.5f), snap(GetRenderFeetY(alpha) - feetOffset), width, height}, source);
        return;
    }
    const float size = static_cast<float>(config::TileSize);
    batch.Draw(atlas, {center.x - 5, center.y + 5, 10, 4}, GetSpriteSource(SpriteId::Shadow));
    const float bob = m_moving ? std::round(std::sin(m_walkTime * 16.0f)) : 0.0f;
    batch.Draw(atlas, {center.x - size * 0.5f, center.y - size * 0.5f + bob, size, size},
               GetSpriteSource(sprite));
}

} // namespace game
