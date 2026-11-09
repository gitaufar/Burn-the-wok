#include "engine/input/Input.h"

#include <SDL/SDL.h>

#include <algorithm>

namespace engine {

namespace {

bool IsValidKey(Key key) {
    const int index = static_cast<int>(key);
    return index >= 0 && index < Input::KeyCount;
}

} // namespace

bool Input::IsKeyDown(Key key) const {
    return IsValidKey(key) && m_keysNow[static_cast<int>(key)];
}

bool Input::IsKeyPressed(Key key) const {
    const int i = static_cast<int>(key);
    return IsValidKey(key) && (m_keysPressed[i] || (m_keysNow[i] && !m_keysPrev[i]));
}

bool Input::IsKeyReleased(Key key) const {
    const int i = static_cast<int>(key);
    return IsValidKey(key) && (m_keysReleased[i] || (!m_keysNow[i] && m_keysPrev[i]));
}

bool Input::IsMouseDown(MouseButton button) const {
    return m_mouseNow[static_cast<int>(button)];
}

bool Input::IsMousePressed(MouseButton button) const {
    const int i = static_cast<int>(button);
    return m_mousePressed[i] || (m_mouseNow[i] && !m_mousePrev[i]);
}

bool Input::IsMouseReleased(MouseButton button) const {
    const int i = static_cast<int>(button);
    return m_mouseReleased[i] || (!m_mouseNow[i] && m_mousePrev[i]);
}

void Input::HandleEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        const int i = static_cast<int>(event.key.keysym.scancode);
        if (i >= 0 && i < KeyCount) {
            if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                m_keysPressed[i] = true;
            } else if (event.type == SDL_KEYUP) {
                m_keysReleased[i] = true;
            }
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
        const int i = event.button.button;
        if (i > 0 && i < MouseButtonCount) {
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                m_mousePressed[i] = true;
            } else {
                m_mouseReleased[i] = true;
            }
        }
    }
    // The wheel has no "state" to poll, only events, so accumulate it here.
    if (event.type == SDL_MOUSEWHEEL) {
        m_mouseWheel += event.wheel.y;
    }
}

void Input::CaptureState() {
    // SDL updates this array while polling events; it is indexed by scancode,
    // which is why Key values are scancodes.
    int sdlKeyCount = 0;
    const Uint8* keys = SDL_GetKeyboardState(&sdlKeyCount);
    const int count = std::min(sdlKeyCount, KeyCount);
    for (int i = 0; i < count; ++i) {
        m_keysNow[i] = keys[i] != 0;
    }

    int mouseX = 0;
    int mouseY = 0;
    const Uint32 buttons = SDL_GetMouseState(&mouseX, &mouseY);
    m_mousePosition = {static_cast<float>(mouseX), static_cast<float>(mouseY)};
    for (int i = 1; i < MouseButtonCount; ++i) {
        m_mouseNow[i] = (buttons & SDL_BUTTON(i)) != 0;
    }
}

void Input::EndUpdate() {
    m_keysPrev = m_keysNow;
    m_mousePrev = m_mouseNow;
    m_keysPressed.fill(false);
    m_keysReleased.fill(false);
    m_mousePressed.fill(false);
    m_mouseReleased.fill(false);
    m_mouseWheel = 0;
}

} // namespace engine
