#include "vte_key_tester.h"
#include "kittykeys.h"
#include <iostream>
#include <cstdio>

#if VTE_GTK == 3
#define VTE_ALT_MASK		GDK_MOD1_MASK
#define VTE_NUMLOCK_MASK	GDK_MOD2_MASK
#elif VTE_GTK == 4
#define VTE_ALT_MASK		GDK_ALT_MASK
#define VTE_NUMLOCK_MASK	0 /* FIXME from VTE source */
#endif

// MockKeyEvent implementation
MockKeyEvent::MockKeyEvent(guint keyval, guint keycode, GdkModifierType modifiers, bool is_press)
    : m_keyval(keyval), m_keycode(keycode), m_modifiers(modifiers), m_is_press(is_press) {}
guint MockKeyEvent::keyval() const { return m_keyval; }
guint MockKeyEvent::keycode() const { return m_keycode; }
GdkModifierType MockKeyEvent::modifiers() const { return m_modifiers; }
bool MockKeyEvent::is_key_press() const { return m_is_press; }

// TesterTerminal implementation
TesterTerminal::TesterTerminal() {
    fprintf(stderr, "[DEBUG] Initializing TesterTerminal...\n");
    m_xkb_data.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!m_xkb_data.context) {
        fprintf(stderr, "[ERROR] xkb_context_new failed.\n");
        m_kitty_keyboard_mode_is_available = false;
        return;
    }
    fprintf(stderr, "[DEBUG] xkb_context_new OK.\n");

    struct xkb_rule_names rules = { .layout = "us" };
    m_xkb_data.keymap_us = xkb_keymap_new_from_names(m_xkb_data.context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!m_xkb_data.keymap_us) {
        fprintf(stderr, "[ERROR] xkb_keymap_new_from_names failed.\n");
        m_kitty_keyboard_mode_is_available = false;
        return;
    }
    fprintf(stderr, "[DEBUG] xkb_keymap_new_from_names OK.\n");

    m_xkb_data.state_us = xkb_state_new(m_xkb_data.keymap_us);
    if (!m_xkb_data.state_us) {
        fprintf(stderr, "[ERROR] xkb_state_new failed.\n");
        m_kitty_keyboard_mode_is_available = false;
        return;
    }
    fprintf(stderr, "[DEBUG] xkb_state_new OK. kitty mode is available.\n");
}

TesterTerminal::~TesterTerminal() {
    if (m_xkb_data.state_us) xkb_state_unref(m_xkb_data.state_us);
    if (m_xkb_data.keymap_us) xkb_keymap_unref(m_xkb_data.keymap_us);
    if (m_xkb_data.context) xkb_context_unref(m_xkb_data.context);
}

void TesterTerminal::send_child(const std::string& seq_str) {
    fprintf(stderr, "[DEBUG] SENDING TO STDOUT: ");
    for(char c : seq_str) {
        if(c == '\x1b') fprintf(stderr, "ESC");
        else if(isprint(c)) fprintf(stderr, "%c", c);
        else fprintf(stderr, "\\x%02x", (unsigned char)c);
    }
    fprintf(stderr, "\n");
    std::cout.write(seq_str.c_str(), seq_str.length());
}

void TesterTerminal::set_kitty_keyboard_flags(int flags) {
    m_kitty_keyboard_flags = flags;
}

bool TesterTerminal::widget_key_press(const MockKeyEvent& event) {
    fprintf(stderr, "\n--- widget_key_press ---\n");
    fprintf(stderr, "PARAMS: Keyval=0x%x, Keycode=%u, KittyFlags=%d, Press=%d\n",
        event.keyval(), event.keycode(), m_kitty_keyboard_flags, event.is_key_press());
    fprintf(stderr, "PRE-CHECK: kitty_flags > 0? %s\n", (m_kitty_keyboard_flags > 0) ? "yes" : "no");
    fprintf(stderr, "PRE-CHECK: kitty_mode_available? %s\n", m_kitty_keyboard_mode_is_available ? "yes" : "no");
    fprintf(stderr, "PRE-CHECK: event.keycode() != 0? %s\n", (event.keycode() != 0) ? "yes" : "no");

#include "vte_key_press_body.inc"

    fprintf(stderr, "[DEBUG] Reached legacy_fallback.\n");
    std::cout << "[LEGACY_FALLBACK]";
    return false;
}