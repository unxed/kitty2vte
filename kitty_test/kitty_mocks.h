#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// --- Defines to satisfy key_encoding.c ---

#define UNUSED __attribute__((unused))
#define KEY_BUFFER_SIZE 128
#define SEND_TEXT_TO_CHILD -2147483648 // INT_MIN

// Macros used in key_encoding.c
#define MAX(a,b) (((a)>(b))?(a):(b))
#define START_ALLOW_CASE_RANGE
#define END_ALLOW_CASE_RANGE

// --- Mocking GLFW constants and types ---

// GLFW Modifiers
#define GLFW_MOD_SHIFT           0x0001
#define GLFW_MOD_CONTROL         0x0002
#define GLFW_MOD_ALT             0x0004
#define GLFW_MOD_SUPER           0x0008
#define GLFW_MOD_CAPS_LOCK       0x0010
#define GLFW_MOD_NUM_LOCK        0x0020
// Extended kitty mods (mapped to GLFW internals usually)
#define GLFW_MOD_HYPER           0x0040
#define GLFW_MOD_META            0x0080

// GLFW Actions
#define GLFW_RELEASE                0
#define GLFW_PRESS                  1
#define GLFW_REPEAT                 2

// Locks mask helper from key_encoding.c logic
#define GLFW_LOCK_MASK (GLFW_MOD_CAPS_LOCK | GLFW_MOD_NUM_LOCK)

// Mock GLFW Structure
typedef struct {
    int key;
    int scancode;
    int action;
    int mods;
    const char* text; // Pointer to UTF8 text
    // kitty extensions to the struct
    uint32_t shifted_key;
    uint32_t alternate_key;
    uint32_t native_key; // Not used in encoding logic usually, but struct presence needed
    int ime_state;
} GLFWkeyevent;

// --- Key Codes ---

enum MockKeys {
    GLFW_FKEY_FIRST = 256,
    GLFW_FKEY_ESCAPE = 256,
    GLFW_FKEY_ENTER,
    GLFW_FKEY_TAB,
    GLFW_FKEY_BACKSPACE,
    GLFW_FKEY_INSERT,
    GLFW_FKEY_DELETE,
    GLFW_FKEY_RIGHT,
    GLFW_FKEY_LEFT,
    GLFW_FKEY_DOWN,
    GLFW_FKEY_UP,
    GLFW_FKEY_PAGE_UP,
    GLFW_FKEY_PAGE_DOWN,
    GLFW_FKEY_HOME,
    GLFW_FKEY_END,
    GLFW_FKEY_CAPS_LOCK,
    GLFW_FKEY_SCROLL_LOCK,
    GLFW_FKEY_NUM_LOCK,
    GLFW_FKEY_PRINT_SCREEN,
    GLFW_FKEY_PAUSE,
    GLFW_FKEY_F1,
    GLFW_FKEY_F2,
    GLFW_FKEY_F3,
    GLFW_FKEY_F4,
    GLFW_FKEY_F5,
    GLFW_FKEY_F6,
    GLFW_FKEY_F7,
    GLFW_FKEY_F8,
    GLFW_FKEY_F9,
    GLFW_FKEY_F10,
    GLFW_FKEY_F11,
    GLFW_FKEY_F12,
    GLFW_FKEY_F13,
    GLFW_FKEY_F14,
    GLFW_FKEY_F15,
    GLFW_FKEY_F16,
    GLFW_FKEY_F17,
    GLFW_FKEY_F18,
    GLFW_FKEY_F19,
    GLFW_FKEY_F20,
    GLFW_FKEY_F21,
    GLFW_FKEY_F22,
    GLFW_FKEY_F23,
    GLFW_FKEY_F24,
    GLFW_FKEY_F25,

    // Keypad numbers
    GLFW_FKEY_KP_0,
    GLFW_FKEY_KP_1,
    GLFW_FKEY_KP_2,
    GLFW_FKEY_KP_3,
    GLFW_FKEY_KP_4,
    GLFW_FKEY_KP_5,
    GLFW_FKEY_KP_6,
    GLFW_FKEY_KP_7,
    GLFW_FKEY_KP_8,
    GLFW_FKEY_KP_9,

    // Keypad math/funcs
    GLFW_FKEY_KP_DECIMAL,
    GLFW_FKEY_KP_DIVIDE,
    GLFW_FKEY_KP_MULTIPLY,
    GLFW_FKEY_KP_SUBTRACT,
    GLFW_FKEY_KP_ADD,
    GLFW_FKEY_KP_ENTER,
    GLFW_FKEY_KP_EQUAL,

    // Keypad navigation
    GLFW_FKEY_KP_HOME,
    GLFW_FKEY_KP_END,
    GLFW_FKEY_KP_INSERT,
    GLFW_FKEY_KP_DELETE,
    GLFW_FKEY_KP_PAGE_UP,
    GLFW_FKEY_KP_PAGE_DOWN,
    GLFW_FKEY_KP_UP,
    GLFW_FKEY_KP_DOWN,
    GLFW_FKEY_KP_LEFT,
    GLFW_FKEY_KP_RIGHT,
    GLFW_FKEY_KP_BEGIN, // Usually numpad 5 without numlock

    // Modifiers
    GLFW_FKEY_LEFT_SHIFT,
    GLFW_FKEY_LEFT_CONTROL,
    GLFW_FKEY_LEFT_ALT,
    GLFW_FKEY_LEFT_SUPER,
    GLFW_FKEY_RIGHT_SHIFT,
    GLFW_FKEY_RIGHT_CONTROL,
    GLFW_FKEY_RIGHT_ALT,
    GLFW_FKEY_RIGHT_SUPER,
    GLFW_FKEY_MENU,

    GLFW_FKEY_KP_SEPARATOR, // Just in case
    GLFW_FKEY_LAST
};

// Modifiers helpers
#define GLFW_FKEY_LEFT_HYPER 0x10001
#define GLFW_FKEY_LEFT_META  0x10002
#define GLFW_FKEY_RIGHT_HYPER 0x10003
#define GLFW_FKEY_RIGHT_META  0x10004
#define GLFW_FKEY_ISO_LEVEL3_SHIFT 0x10005
#define GLFW_FKEY_ISO_LEVEL5_SHIFT 0x10006


// --- Mocking charsets.h functionality ---

typedef uint32_t UTF8State;
const UTF8State UTF8_ACCEPT = 0;
const UTF8State UTF8_REJECT = 1;

static inline UTF8State decode_utf8(UTF8State* state, uint32_t* codep, uint32_t byte) {
    uint32_t b = byte & 0xFF;
    if (*state == UTF8_ACCEPT) {
        if (b < 0x80) { *codep = b; return UTF8_ACCEPT; }
        if ((b & 0xE0) == 0xC0) { *codep = b & 0x1F; *state = 1; return UTF8_REJECT; }
        if ((b & 0xF0) == 0xE0) { *codep = b & 0x0F; *state = 2; return UTF8_REJECT; }
        if ((b & 0xF8) == 0xF0) { *codep = b & 0x07; *state = 3; return UTF8_REJECT; }
        return UTF8_REJECT;
    } else {
        if ((b & 0xC0) != 0x80) { *state = UTF8_ACCEPT; return UTF8_REJECT; }
        *codep = (*codep << 6) | (b & 0x3F);
        (*state)--;
        if (*state == 0) return UTF8_ACCEPT;
        return UTF8_REJECT;
    }
}

static inline int encode_utf8(uint32_t codepoint, char* output) {
    if (codepoint < 0x80) {
        output[0] = (char)codepoint;
        output[1] = 0;
        return 1;
    }
    return 0;
}

// --- Mocking keys.h functionality ---

static bool is_modifier_key(uint32_t key) {
    switch (key) {
        case GLFW_FKEY_LEFT_SHIFT:
        case GLFW_FKEY_RIGHT_SHIFT:
        case GLFW_FKEY_LEFT_CONTROL:
        case GLFW_FKEY_RIGHT_CONTROL:
        case GLFW_FKEY_LEFT_ALT:
        case GLFW_FKEY_RIGHT_ALT:
        case GLFW_FKEY_LEFT_SUPER:
        case GLFW_FKEY_RIGHT_SUPER:
        case GLFW_FKEY_CAPS_LOCK:
        case GLFW_FKEY_NUM_LOCK:
        case GLFW_FKEY_SCROLL_LOCK:
        case GLFW_FKEY_LEFT_HYPER:
        case GLFW_FKEY_RIGHT_HYPER:
        case GLFW_FKEY_LEFT_META:
        case GLFW_FKEY_RIGHT_META:
        case GLFW_FKEY_ISO_LEVEL3_SHIFT:
        case GLFW_FKEY_ISO_LEVEL5_SHIFT:
            return true;
        default:
            return false;
    }
}