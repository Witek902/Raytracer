#pragma once

#ifdef WIN32

enum class MouseButton : Uint32
{
    Left = 0,
    Middle = 2,
    Right = 1,
};

/**
 * KeyCode enum for Windows. Based on official MSDN reference:
 *   https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
 */
enum class KeyCode : Uint32
{
    Invalid = 0,
    Backspace = VK_BACK,
    Tab = VK_TAB,
    // here 0x0A-0x0B are reserved, 0x0C is unused CLEAR
    Enter = VK_RETURN,
    // 0x0E - 0x0F reserved
    Shift = VK_SHIFT,
    Control,
    Alt,
    Pause = VK_PAUSE,
    CapsLock,
    // 0x15 - 0x1A are IME-related keys, not needed by us
    Escape = VK_ESCAPE,
    // 0x1C - 0x1F are IME-related keys
    Space = VK_SPACE,
    PageUp,
    PageDown,
    End,
    Home,
    Left,
    Up,
    Right,
    Down,
    Select,
    Print,
    Execute,
    PrintScreen,
    Insert,
    Delete,
    // Help button is deprecated
    Num0 = '0',
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    // 0x3A - 0x40 are undefined
    A = 'A',
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    WinLeft,
    WinRight,
    // 0x5D - Apps (??), 0x5E - Reserved, 0x5F - Sleep
    Numpad0 = VK_NUMPAD0,
    Numpad1,
    Numpad2,
    Numpad3,
    Numpad4,
    Numpad5,
    Numpad6,
    Numpad7,
    Numpad8,
    Numpad9,
    NumpadMulitply,
    NumpadAdd,
    NumpadSeparator,
    NumpadSubtract,
    NumpadDecimal,
    NumpadDivide,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    // Windows supports up to F24, but it is rarely seen and used nowadays
    // 0x88-0x8F - unassigned keys
    NumLock = VK_NUMLOCK,
    ScrollLock,
    // 0x92-0x96 - OEM-specific, 0x97-0x9F - unassigned
    ShiftLeft = VK_LSHIFT,
    ShiftRight,
    ControlLeft,
    ControlRight,
    AltLeft,
    AltRight,
    // Here should be some media-related keys (play/pause, volume ctl, etc)
    // Add them if needed based on MSDN reference link above
    Semicolon = VK_OEM_1,
    Plus,
    Comma,
    Minus,
    Period,
    Slash,
    Tilde,
    // 0xC1-0xDA are reserved/unassigned
    BracketOpen = VK_OEM_4,
    Backslash,
    BracketClose,
    Quote
};

#elif defined(__linux__) || defined(__LINUX__)

enum class MouseButton : Uint32
{
    Left = 0,
    Middle = 1,
    Right = 2,
};

/**
 * KeyCode enum for Linux. Based on reports coming from xev.
 * For verification, it is recommended to launch with following call:
 *   xev | awk -F'[ )]+' '/^KeyPress/ { a[NR+2] } NR in a { printf "%-3s %s\n", $5, $8 }'
 */
enum class KeyCode : Uint32
{
    Invalid = 0,
    Escape = 9,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    Num0,
    Minus,
    Equal,
    Backspace,
    Tab,
    Q,
    W,
    E,
    R,
    T,
    Y,
    U,
    I,
    O,
    P,
    BracketOpen,
    BracketClose,
    Enter,
    ControlLeft,
    A,
    S,
    D,
    F,
    G,
    H,
    J,
    K,
    L,
    Semicolon,
    Quote,
    Tilde,
    ShiftLeft,
    Backslash,
    Z,
    X,
    C,
    V,
    B,
    N,
    M,
    Comma,
    Period,
    Slash,
    ShiftRight,
    NumpadMultiply,
    AltLeft,
    Space,
    CapsLock,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    NumLock,
    ScrollLock,
    Numpad7,
    Numpad8,
    Numpad9,
    NumpadSubtract,
    Numpad4,
    Numpad5,
    Numpad6,
    NumpadAdd,
    Numpad1,
    Numpad2,
    Numpad3,
    Numpad0,
    NumpadDecimal,
    // no idea what is hidden in between here
    F11 = 95,
    F12,
    // no idea what is hidden in between here
    NumpadEnter = 104,
    // no idea what is hidden in between here
    NumpadDivide = 106,
    // no idea what is hidden in between here
    Home = 110,
    Up,
    PageUp,
    Left,
    Right,
    End,
    Down,
    PageDown,
    Insert,
    Delete,
    // no idea what is hidden in between here
    Pause = 127,
    // no idea what is hidden in between here
    WinLeft = 133,
    WinRight,

    // to comply with Windows, make Shift/Alt/Control keys the same as left equivalents
    Shift = ShiftLeft,
    Alt = AltLeft,
    Control = ControlLeft
};

#else
#error Target platform not supported.
#endif
