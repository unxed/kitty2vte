#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <xkbcommon/xkbcommon.h>

typedef unsigned int guint;
typedef unsigned int GdkModifierType;

#define GDK_SHIFT_MASK    (1<<0)
#define GDK_LOCK_MASK     (1<<1)
#define GDK_CONTROL_MASK  (1<<2)
#define GDK_MOD1_MASK     (1<<3)  // Alt
#define GDK_MOD2_MASK     (1<<4)  // Num Lock
#define GDK_MOD3_MASK     (1<<5)
#define GDK_MOD4_MASK     (1<<6)  // Super
#define GDK_MOD5_MASK     (1<<7)  // Hyper
#define GDK_SUPER_MASK    GDK_MOD4_MASK
#define GDK_HYPER_MASK    GDK_MOD5_MASK
#define GDK_META_MASK     (1<<28) 

// VTE uses GDK_MOD1_MASK for Alt
#define VTE_ALT_MASK		GDK_MOD1_MASK
// In GTK4, GDK_ALT_MASK is absent, GDK_MOD1_MASK is used
#define GDK_ALT_MASK        GDK_MOD1_MASK 
// VTE defines VTE_NUMLOCK_MASK as GDK_MOD2_MASK in gtk3 and as 0 in gtk4,
// but code we extracted uses VTE_NUMLOCK_MASK, so lets define it
#define VTE_NUMLOCK_MASK	GDK_MOD2_MASK

// --- GDK Key Symbols (from gdkkeysyms.h) ---
enum {
    GDK_KEY_a = 0x0061,
    GDK_KEY_z = 0x007a,
    GDK_KEY_0 = 0x0030,
    GDK_KEY_9 = 0x0039,
    GDK_KEY_minus = 0x002d,
    GDK_KEY_equal = 0x003d,
    GDK_KEY_space = 0x0020,
    GDK_KEY_Escape = 0xff1b,
    GDK_KEY_Tab = 0xff09,
    GDK_KEY_Return = 0xff0d,
    GDK_KEY_BackSpace = 0xff08,
    GDK_KEY_Home = 0xff50,
    GDK_KEY_End = 0xff57,
    GDK_KEY_Up = 0xff52,
    GDK_KEY_Down = 0xff54,
    GDK_KEY_Left = 0xff51,
    GDK_KEY_Right = 0xff53,
    GDK_KEY_Insert = 0xff63,
    GDK_KEY_Delete = 0xffff,
    GDK_KEY_Page_Up = 0xff55,
    GDK_KEY_Page_Down = 0xff56,
    GDK_KEY_bracketleft = 0x005b,
    GDK_KEY_bracketright = 0x005d,
    GDK_KEY_backslash = 0x005c,
    GDK_KEY_semicolon = 0x003b,
    GDK_KEY_apostrophe = 0x0027,
    GDK_KEY_comma = 0x002c,
    GDK_KEY_period = 0x002e,
    GDK_KEY_slash = 0x002f,
    GDK_KEY_grave = 0x0060,
    
    // Function keys
    GDK_KEY_F1 = 0xffbe, GDK_KEY_F2 = 0xffbf, GDK_KEY_F3 = 0xffc0,
    GDK_KEY_F4 = 0xffc1, GDK_KEY_F5 = 0xffc2, GDK_KEY_F6 = 0xffc3,
    GDK_KEY_F7 = 0xffc4, GDK_KEY_F8 = 0xffc5, GDK_KEY_F9 = 0xffc6,
    GDK_KEY_F10 = 0xffc7, GDK_KEY_F11 = 0xffc8, GDK_KEY_F12 = 0xffc9,

    // Misc function keys
    GDK_KEY_Scroll_Lock = 0xFF14,
    GDK_KEY_Num_Lock = 0xFF7F,
    GDK_KEY_Caps_Lock = 0xFFE5,
    GDK_KEY_Print = 0xFF61,
    GDK_KEY_Pause = 0xFF13,
    GDK_KEY_Menu = 0xFF67,
    
    // Modifier keys
    GDK_KEY_Shift_L = 0xFFE1,
    GDK_KEY_Shift_R = 0xFFE2,
    GDK_KEY_Control_L = 0xFFE3,
    GDK_KEY_Control_R = 0xFFE4,
    GDK_KEY_Alt_L = 0xFFE9,
    GDK_KEY_Alt_R = 0xFFEA,
    GDK_KEY_Meta_L = 0xFFE7, // Super_L
    GDK_KEY_Meta_R = 0xFFE8, // Super_R
    GDK_KEY_ISO_Level3_Shift = 0xFE03,

    // Keypad keys
    GDK_KEY_KP_0 = 0xffb0, GDK_KEY_KP_1 = 0xffb1, GDK_KEY_KP_2 = 0xffb2,
    GDK_KEY_KP_3 = 0xffb3, GDK_KEY_KP_4 = 0xffb4, GDK_KEY_KP_5 = 0xffb5,
    GDK_KEY_KP_6 = 0xffb6, GDK_KEY_KP_7 = 0xffb7, GDK_KEY_KP_8 = 0xffb8,
    GDK_KEY_KP_9 = 0xffb9,
    GDK_KEY_KP_Decimal = 0xffae,
    GDK_KEY_KP_Divide = 0xffaf,
    GDK_KEY_KP_Multiply = 0xffaa,
    GDK_KEY_KP_Subtract = 0xffad,
    GDK_KEY_KP_Add = 0xffab,
    GDK_KEY_KP_Enter = 0xff8d,
    GDK_KEY_KP_Equal = 0xffbd,
    GDK_KEY_KP_Separator = 0xffac,
    GDK_KEY_KP_Left = 0xff96,
    GDK_KEY_KP_Right = 0xff98,
    GDK_KEY_KP_Up = 0xff97,
    GDK_KEY_KP_Down = 0xff99,
    GDK_KEY_KP_Page_Up = 0xff9a,
    GDK_KEY_KP_Page_Down = 0xff9b,
    GDK_KEY_KP_Home = 0xff95,
    GDK_KEY_KP_End = 0xff9c,
    GDK_KEY_KP_Insert = 0xff9e,
    GDK_KEY_KP_Delete = 0xff9f,
    GDK_KEY_KP_Begin = 0xff9d
};

static inline guint gdk_keyval_to_unicode(guint keyval) {
    if (keyval >= 0x0020 && keyval <= 0x007e) return keyval; // ASCII
    if (keyval >= 0x00a0 && keyval <= 0x00ff) return keyval; // Latin-1
    // Simplified stub for tests
    return keyval;
}
static inline guint gdk_keyval_to_lower(guint keyval) {
    if (keyval >= 'A' && keyval <= 'Z') return keyval + ('a' - 'A');
    return keyval;
}
static inline guint gdk_keyval_to_upper(guint keyval) {
    if (keyval >= 'a' && keyval <= 'z') return keyval - ('a' - 'A');
    return keyval;
}

class MockKeyEvent {
public:
    MockKeyEvent(guint keyval, guint keycode, GdkModifierType modifiers, bool is_press);
    guint keyval() const;
    guint keycode() const;
    GdkModifierType modifiers() const;
    bool is_key_press() const;
private:
    guint m_keyval;
    guint m_keycode;
    GdkModifierType m_modifiers;
    bool m_is_press;
};

class TesterTerminal {
public:
    struct XkbData {
        struct xkb_context *context = nullptr;
        struct xkb_keymap *keymap_us = nullptr;
        struct xkb_state *state_us = nullptr;
    };
    XkbData m_xkb_data;
    int m_kitty_keyboard_flags = 0;
    std::unordered_set<guint> m_active_keys;
    bool m_kitty_keyboard_mode_is_available = true;

    TesterTerminal();
    ~TesterTerminal();
    void send_child(const std::string& seq_str);
    void set_kitty_keyboard_flags(int flags);
    bool widget_key_press(const MockKeyEvent& event);
};