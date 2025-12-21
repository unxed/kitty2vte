#include "kitty_mocks.h"
#include "kitty_encoder_body.inc"
#include <string.h>
#include <ctype.h>

typedef struct {
    const char* name;
    int key;
    int shifted_key;
    const char* numpad_text;
} KeyInfo;

static const KeyInfo key_map[] = {
    // Letters
    {"a", 'a', 'A', NULL}, {"b", 'b', 'B', NULL}, {"c", 'c', 'C', NULL},
    {"d", 'd', 'D', NULL}, {"e", 'e', 'E', NULL}, {"f", 'f', 'F', NULL},
    {"g", 'g', 'G', NULL}, {"h", 'h', 'H', NULL}, {"i", 'i', 'I', NULL},
    {"j", 'j', 'J', NULL}, {"k", 'k', 'K', NULL}, {"l", 'l', 'L', NULL},
    {"m", 'm', 'M', NULL}, {"n", 'n', 'N', NULL}, {"o", 'o', 'O', NULL},
    {"p", 'p', 'P', NULL}, {"q", 'q', 'Q', NULL}, {"r", 'r', 'R', NULL},
    {"s", 's', 'S', NULL}, {"t", 't', 'T', NULL}, {"u", 'u', 'U', NULL},
    {"v", 'v', 'V', NULL}, {"w", 'w', 'W', NULL}, {"x", 'x', 'X', NULL},
    {"y", 'y', 'Y', NULL}, {"z", 'z', 'Z', NULL},
    // Numbers
    {"0", '0', ')', NULL}, {"1", '1', '!', NULL}, {"2", '2', '@', NULL},
    {"3", '3', '#', NULL}, {"4", '4', '$', NULL}, {"5", '5', '%', NULL},
    {"6", '6', '^', NULL}, {"7", '7', '&', NULL}, {"8", '8', '*', NULL},
    {"9", '9', '(', NULL},
    // Symbols & Aliases for run_tests.py
    {"`", '`', '~', NULL}, {"~", '`', '~', NULL},
    {"-", '-', '_', NULL}, {"_", '-', '_', NULL}, {"minus", '-', '_', NULL},
    {"=", '=', '+', NULL}, {"+", '=', '+', NULL}, {"equal", '=', '+', NULL},
    {"[", '[', '{', NULL}, {"{", '[', '{', NULL}, {"bracketleft", '[', '{', NULL},
    {"]", ']', '}', NULL}, {"}", ']', '}', NULL}, {"bracketright", ']', '}', NULL},
    {"\\", '\\', '|', NULL}, {"|", '\\', '|', NULL}, {"backslash", '\\', '|', NULL},
    {";", ';', ':', NULL}, {":", ';', ':', NULL}, {"semicolon", ';', ':', NULL},
    {"'", '\'', '"', NULL}, {"\"", '\'', '"', NULL}, {"apostrophe", '\'', '"', NULL},
    {",", ',', '<', NULL}, {"<", ',', '<', NULL}, {"comma", ',', '<', NULL},
    {".", '.', '>', NULL}, {">", '.', '>', NULL}, {"period", '.', '>', NULL},
    {"/", '/', '?', NULL}, {"?", '/', '?', NULL}, {"slash", '/', '?', NULL},
    // Functional Keys
    {"F1", GLFW_FKEY_F1, 0, NULL}, {"F2", GLFW_FKEY_F2, 0, NULL},
    {"F3", GLFW_FKEY_F3, 0, NULL}, {"F4", GLFW_FKEY_F4, 0, NULL},
    {"F5", GLFW_FKEY_F5, 0, NULL}, {"F6", GLFW_FKEY_F6, 0, NULL},
    {"F7", GLFW_FKEY_F7, 0, NULL}, {"F8", GLFW_FKEY_F8, 0, NULL},
    {"F9", GLFW_FKEY_F9, 0, NULL}, {"F10", GLFW_FKEY_F10, 0, NULL},
    {"F11", GLFW_FKEY_F11, 0, NULL}, {"F12", GLFW_FKEY_F12, 0, NULL},
    // Control keys
    {"Escape", GLFW_FKEY_ESCAPE, 0, NULL},
    {"Tab", GLFW_FKEY_TAB, 0, NULL},
    {"Return", GLFW_FKEY_ENTER, 0, NULL},
    {"BackSpace", GLFW_FKEY_BACKSPACE, 0, NULL},
    {"space", ' ', ' ', NULL},
    // Navigation
    {"Insert", GLFW_FKEY_INSERT, 0, NULL},
    {"Delete", GLFW_FKEY_DELETE, 0, NULL},
    {"Home", GLFW_FKEY_HOME, 0, NULL},
    {"End", GLFW_FKEY_END, 0, NULL},
    {"Page_Up", GLFW_FKEY_PAGE_UP, 0, NULL},
    {"Page_Down", GLFW_FKEY_PAGE_DOWN, 0, NULL},
    // Arrows
    {"Up", GLFW_FKEY_UP, 0, NULL}, {"Down", GLFW_FKEY_DOWN, 0, NULL},
    {"Left", GLFW_FKEY_LEFT, 0, NULL}, {"Right", GLFW_FKEY_RIGHT, 0, NULL},
    // Keypad
    {"KP_0", 57399, 0, "0"}, {"KP_1", 57400, 0, "1"},
    {"KP_2", 57401, 0, "2"}, {"KP_3", 57402, 0, "3"},
    {"KP_4", 57403, 0, "4"}, {"KP_5", 57404, 0, "5"},
    {"KP_6", 57405, 0, "6"}, {"KP_7", 57406, 0, "7"},
    {"KP_8", 57407, 0, "8"}, {"KP_9", 57408, 0, "9"},
    {"KP_Decimal", 57409, 0, "."}, {"KP_Divide", 57410, 0, "/"},
    {"KP_Multiply", 57411, 0, "*"}, {"KP_Subtract", 57412, 0, "-"},
    {"KP_Add", 57413, 0, "+"}, {"KP_Enter", 57414, 0, "\r"},
    {"KP_Equal", 57415, 0, "="}, {"KP_Separator", 57416, 0, ","},
    {"KP_Left", 57417, 0, NULL}, {"KP_Right", 57418, 0, NULL},
    {"KP_Up", 57419, 0, NULL}, {"KP_Down", 57420, 0, NULL},
    {"KP_Page_Up", 57421, 0, NULL}, {"KP_Page_Down", 57422, 0, NULL},
    {"KP_Home", 57423, 0, NULL}, {"KP_End", 57424, 0, NULL},
    {"KP_Insert", 57425, 0, NULL}, {"KP_Delete", 57426, 0, NULL},
    {"KP_Begin", 57427, 0, NULL},
    // Non-English (1103 = 'я', 1071 = 'Я')
    {"я", 1103, 1071, NULL},
    {NULL, 0, 0, NULL}
};

// Encodes a single Unicode codepoint into a UTF-8 string buffer
static void encode_codepoint_to_utf8(uint32_t cp, char* buf) {
    if (cp < 0x80) {
        buf[0] = cp;
        buf[1] = '\0';
    } else if (cp < 0x800) {
        buf[0] = 0xC0 | (cp >> 6);
        buf[1] = 0x80 | (cp & 0x3F);
        buf[2] = '\0';
    } else if (cp < 0x10000) {
        buf[0] = 0xE0 | (cp >> 12);
        buf[1] = 0x80 | ((cp >> 6) & 0x3F);
        buf[2] = 0x80 | (cp & 0x3F);
        buf[3] = '\0';
    } else {
        buf[0] = 0xF0 | (cp >> 18);
        buf[1] = 0x80 | ((cp >> 12) & 0x3F);
        buf[2] = 0x80 | ((cp >> 6) & 0x3F);
        buf[3] = 0x80 | (cp & 0x3F);
        buf[4] = '\0';
    }
}
static const KeyInfo* find_key_info(const char* name) {
    for (int i = 0; key_map[i].name != NULL; i++) {
        if (strcmp(key_map[i].name, name) == 0) {
            return &key_map[i];
        }
    }
    return NULL;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s --key <Name> [--shift] [--ctrl] [--alt] [--super] [--caps] [--num] [--kitty-flags <int>] [--action <press|release|repeat>] [--cursor-key-mode]\n", argv[0]);
        return 1;
    }

    GLFWkeyevent ev;
    memset(&ev, 0, sizeof(ev));
    ev.action = GLFW_PRESS; // Default

    const char* key_name = NULL;
    unsigned int kitty_flags = 0;
    bool cursor_key_mode = false;
    bool has_mods_that_prevent_text = false;
    const char* base_key_str = NULL;

    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];
        if (strcmp(arg, "--key") == 0 && i + 1 < argc) {
            key_name = argv[++i];
        }
        else if (strcmp(arg, "--shift") == 0) ev.mods |= GLFW_MOD_SHIFT;
        else if (strcmp(arg, "--ctrl") == 0) { ev.mods |= GLFW_MOD_CONTROL; has_mods_that_prevent_text = true; }
        else if (strcmp(arg, "--alt") == 0) { ev.mods |= GLFW_MOD_ALT; has_mods_that_prevent_text = true; }
        else if (strcmp(arg, "--super") == 0) { ev.mods |= GLFW_MOD_SUPER; has_mods_that_prevent_text = true; }
        else if (strcmp(arg, "--caps") == 0) ev.mods |= GLFW_MOD_CAPS_LOCK;
        else if (strcmp(arg, "--num") == 0) ev.mods |= GLFW_MOD_NUM_LOCK;
        else if (strcmp(arg, "--action") == 0 && i + 1 < argc) {
            const char* action_str = argv[++i];
            if (strcmp(action_str, "release") == 0) ev.action = GLFW_RELEASE;
            else if (strcmp(action_str, "repeat") == 0) ev.action = GLFW_REPEAT;
        }
        else if (strcmp(arg, "--kitty-flags") == 0 && i + 1 < argc) {
            kitty_flags = atoi(argv[++i]);
        }
        else if (strcmp(arg, "--cursor-key-mode") == 0) {
            cursor_key_mode = true;
        }
        else if (strcmp(arg, "--base-key") == 0 && i + 1 < argc) {
            base_key_str = argv[++i];
        }
    }

    if (!key_name) {
        fprintf(stderr, "Error: --key argument is missing.\n");
        return 1;
    }

    const KeyInfo* key_info = find_key_info(key_name);
    if (!key_info) {
        fprintf(stderr, "Error: Unknown key name '%s'.\n", key_name);
        return 1;
    }
    ev.key = key_info->key;
    ev.shifted_key = key_info->shifted_key;

    // Apply Caps Lock effect to shifted_key
    // If Caps Lock is on, the "Shifted" version of a letter is lowercase
    if ((ev.mods & GLFW_MOD_CAPS_LOCK) && ev.key >= 'a' && ev.key <= 'z') {
        if (ev.shifted_key >= 'A' && ev.shifted_key <= 'Z') {
             ev.shifted_key = ev.shifted_key + ('a' - 'A');
        }
    }

    // If shifted_key is same as key (e.g. Shift+Caps+a -> 'a', key is 'a'),
    // it provides no info and should be 0 to match real behavior
    if (ev.shifted_key == ev.key) {
        ev.shifted_key = 0;
    }

    static char text_buf[8] = {0};
    bool is_function_key = (key_info->key >= GLFW_FKEY_FIRST && key_info->key <= GLFW_FKEY_LAST);
    if (!has_mods_that_prevent_text && !is_function_key) {
        bool shift_active = (ev.mods & GLFW_MOD_SHIFT) != 0;
        bool caps_active = (ev.mods & GLFW_MOD_CAPS_LOCK) != 0;
        bool effective_shift = shift_active ^ caps_active;

        // A printable unicode character, but not a PUA functional key
        if (key_info->key > 127 && key_info->key < 57344) {
            uint32_t codepoint = effective_shift ? key_info->shifted_key : key_info->key;
            encode_codepoint_to_utf8(codepoint, text_buf);
            ev.text = text_buf;
        } else if (key_info->key >= 'a' && key_info->key <= 'z') {
            text_buf[0] = effective_shift ? (char)key_info->shifted_key : (char)key_info->key;
            text_buf[1] = '\0';
            ev.text = text_buf;
        } else if (key_info->shifted_key != 0) {
            text_buf[0] = shift_active ? (char)key_info->shifted_key : (char)key_info->key;
            text_buf[1] = '\0';
            ev.text = text_buf;
        } else if (key_info->key < 256) {
             text_buf[0] = (char)key_info->key;
             text_buf[1] = '\0';
             ev.text = text_buf;
        }

        if (key_info->numpad_text && (ev.mods & GLFW_MOD_NUM_LOCK)) {
             text_buf[0] = key_info->numpad_text[0];
             text_buf[1] = '\0';
             ev.text = text_buf;
        }
    }

    // Set alternate_key (Base Layout Key)
    if (base_key_str && base_key_str[0]) {
        ev.alternate_key = base_key_str[0];
    } else {
        // Default base key logic for ASCII when not provided
        uint32_t potential_alt = 0;
        if (ev.key < 128) {
            if (ev.key >= 'A' && ev.key <= 'Z') potential_alt = ev.key + 32;
            else if (ev.key >= 'a' && ev.key <= 'z') potential_alt = ev.key;
            // For other ASCII like punctuation, base key is the key itself, so we don't set it unless different
        }
        if (potential_alt != 0 && potential_alt != ev.key) {
            ev.alternate_key = potential_alt;
        }
    }

    char output[KEY_BUFFER_SIZE];
    memset(output, 0, KEY_BUFFER_SIZE);

    int result = encode_glfw_key_event(&ev, cursor_key_mode, kitty_flags, output);

    if (result == SEND_TEXT_TO_CHILD) {
        if (ev.text) {
            fwrite(ev.text, 1, strlen(ev.text), stdout);
        }
    } else if (result > 0) {
        fwrite(output, 1, result, stdout);
    }

    fprintf(stderr, "[kittyTester] Key: %u, Shifted: %u, Mods: %d, Flags: %d, Action: %d, Text: '%s' -> Result Len: %d\n",
            ev.key, ev.shifted_key, ev.mods, kitty_flags, ev.action, ev.text ? ev.text : "(null)", result);

    return 0;
}