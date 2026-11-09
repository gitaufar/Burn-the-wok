#pragma once

namespace engine {

// Physical key positions. The numeric values are the USB HID usage codes, which are
// also what SDL uses for its scancodes, so the input system can index SDL's keyboard
// state array directly. Physical means layout-independent: Key::W is "the key where W
// sits on a QWERTY keyboard", which is what WASD movement wants.
enum class Key : int {
    A = 4, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z, // Z = 29

    Num1 = 30, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0, // Num0 = 39

    Enter = 40,
    Escape = 41,
    Backspace = 42,
    Tab = 43,
    Space = 44,

    F1 = 58, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, // F12 = 69

    Right = 79,
    Left = 80,
    Down = 81,
    Up = 82,
    NumpadEnter = 88,

    LeftCtrl = 224,
    LeftShift = 225,
    LeftAlt = 226,
    RightCtrl = 228,
    RightShift = 229,
    RightAlt = 230,
};

// Values match SDL_BUTTON_LEFT / MIDDLE / RIGHT.
enum class MouseButton : int {
    Left = 1,
    Middle = 2,
    Right = 3,
};

} // namespace engine
