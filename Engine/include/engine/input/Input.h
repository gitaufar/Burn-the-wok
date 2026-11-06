#pragma once

#include "engine/input/Keys.h"
#include "engine/math/Vec2.h"

#include <array>

union SDL_Event;

namespace engine {

// Keyboard and mouse state, sampled once per frame by Application.
//
//   IsKeyDown     -> held right now
//   IsKeyPressed  -> went down since the previous fixed update (true for exactly one update)
//   IsKeyReleased -> went up since the previous fixed update
//
// "Previous" is tied to fixed updates, not rendered frames, so a press is never seen
// twice when several updates run in one frame, and never missed when a frame runs no update.
class Input {
public:
    static constexpr int KeyCount = 512; // same as SDL_NUM_SCANCODES

    bool IsKeyDown(Key key) const;
    bool IsKeyPressed(Key key) const;
    bool IsKeyReleased(Key key) const;

    bool IsMouseDown(MouseButton button) const;
    bool IsMousePressed(MouseButton button) const;
    bool IsMouseReleased(MouseButton button) const;

    // Window pixels, origin at the top-left of the window.
    Vec2 GetMousePosition() const { return m_mousePosition; }
    // Wheel notches since the previous fixed update; positive = away from the user.
    int GetMouseWheel() const { return m_mouseWheel; }

private:
    friend class Application;

    void HandleEvent(const SDL_Event& event); // per OS event
    void CaptureState();                      // once per frame, after all events are polled
    void EndUpdate();                         // after every fixed update

    static constexpr int MouseButtonCount = 8;

    std::array<bool, KeyCount> m_keysNow{};
    std::array<bool, KeyCount> m_keysPrev{};
    // Keep short taps until a fixed update consumes them, even if down and up
    // both arrive before SDL_GetKeyboardState is sampled.
    std::array<bool, KeyCount> m_keysPressed{};
    std::array<bool, KeyCount> m_keysReleased{};
    std::array<bool, MouseButtonCount> m_mouseNow{};
    std::array<bool, MouseButtonCount> m_mousePrev{};
    std::array<bool, MouseButtonCount> m_mousePressed{};
    std::array<bool, MouseButtonCount> m_mouseReleased{};
    Vec2 m_mousePosition{};
    int m_mouseWheel = 0;
};

} // namespace engine
