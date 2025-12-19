#include "far2l_mocks.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

// Include the extracted logic
// The function signature matches the one found in source/vtshell_translation_kitty.cpp
std::string VT_TranslateKeyToKitty(const KEY_EVENT_RECORD &KeyEvent, int flags, unsigned char keypad)
{
#include "far2l_key_press_body.inc"
    // Fallback if the extracted body falls through (shouldn't happen usually with return statements)
    return "";
}

struct KeyDef {
    WORD vk;
    WCHAR ch;
    WCHAR shift_ch; // Character produced when Shift is pressed (simple emulation)
};

std::map<std::string, KeyDef> key_map;

void init_key_map() {
    // Letters
    for (char c = 'a'; c <= 'z'; ++c) {
        WCHAR wc = (WCHAR)c;
        WCHAR upper = (WCHAR)toupper(c);
        key_map[std::string(1, c)] = { (WORD)toupper(c), wc, upper };
    }
    // Numbers
    std::string numbers = "0123456789";
    std::string shift_nums = ")!@#$%^&*(";
    for (int i = 0; i < 10; ++i) {
        WORD vk = 0x30 + i;
        key_map[numbers.substr(i, 1)] = { vk, (WCHAR)numbers[i], (WCHAR)shift_nums[i] };
    }

    // Function keys
    for (int i = 1; i <= 12; ++i) {
        key_map["F" + std::to_string(i)] = { (WORD)(VK_F1 + i - 1), 0, 0 };
    }

    // Control & Nav
    key_map["Escape"] = { VK_ESCAPE, 0, 0 };
    key_map["Tab"] = { VK_TAB, '\t', '\t' };
    key_map["Return"] = { VK_RETURN, '\r', '\r' };
    key_map["BackSpace"] = { VK_BACK, '\x08', '\x08' };
    key_map["space"] = { VK_SPACE, ' ', ' ' };

    key_map["Insert"] = { VK_INSERT, 0, 0 };
    key_map["Delete"] = { VK_DELETE, 0, 0 };
    key_map["Home"] = { VK_HOME, 0, 0 };
    key_map["End"] = { VK_END, 0, 0 };
    key_map["Page_Up"] = { VK_PRIOR, 0, 0 };
    key_map["Page_Down"] = { VK_NEXT, 0, 0 };
    key_map["Up"] = { VK_UP, 0, 0 };
    key_map["Down"] = { VK_DOWN, 0, 0 };
    key_map["Left"] = { VK_LEFT, 0, 0 };
    key_map["Right"] = { VK_RIGHT, 0, 0 };

    // Symbols
    // Maps based on US Standard Keyboard Layout
    key_map["`"] = { VK_OEM_3, '`', '~' };
    key_map["~"] = { VK_OEM_3, '`', '~' };
    key_map["-"] = { VK_OEM_MINUS, '-', '_' };
    key_map["minus"] = { VK_OEM_MINUS, '-', '_' };
    key_map["_"] = { VK_OEM_MINUS, '-', '_' };
    key_map["="] = { VK_OEM_PLUS, '=', '+' };
    key_map["equal"] = { VK_OEM_PLUS, '=', '+' };
    key_map["+"] = { VK_OEM_PLUS, '=', '+' };

    key_map["["] = { VK_OEM_4, '[', '{' };
    key_map["bracketleft"] = { VK_OEM_4, '[', '{' };
    key_map["{"] = { VK_OEM_4, '[', '{' };

    key_map["]"] = { VK_OEM_6, ']', '}' };
    key_map["bracketright"] = { VK_OEM_6, ']', '}' };
    key_map["}"] = { VK_OEM_6, ']', '}' };

    key_map["\\"] = { VK_OEM_5, '\\', '|' };
    key_map["backslash"] = { VK_OEM_5, '\\', '|' };
    key_map["|"] = { VK_OEM_5, '\\', '|' };

    key_map[";"] = { VK_OEM_1, ';', ':' };
    key_map["semicolon"] = { VK_OEM_1, ';', ':' };
    key_map[":"] = { VK_OEM_1, ';', ':' };

    key_map["'"] = { VK_OEM_7, '\'', '"' };
    key_map["apostrophe"] = { VK_OEM_7, '\'', '"' };
    key_map["\""] = { VK_OEM_7, '\'', '"' };

    key_map[","] = { VK_OEM_COMMA, ',', '<' };
    key_map["comma"] = { VK_OEM_COMMA, ',', '<' };
    key_map["<"] = { VK_OEM_COMMA, ',', '<' };

    key_map["."] = { VK_OEM_PERIOD, '.', '>' };
    key_map["period"] = { VK_OEM_PERIOD, '.', '>' };
    key_map[">"] = { VK_OEM_PERIOD, '.', '>' };

    key_map["/"] = { VK_OEM_2, '/', '?' };
    key_map["slash"] = { VK_OEM_2, '/', '?' };
    key_map["?"] = { VK_OEM_2, '/', '?' };

    // Keypad
    key_map["KP_0"] = { VK_NUMPAD0, 0, 0 }; // With NumLock these produce digits
    key_map["KP_1"] = { VK_NUMPAD1, 0, 0 };
    key_map["KP_2"] = { VK_NUMPAD2, 0, 0 };
    key_map["KP_3"] = { VK_NUMPAD3, 0, 0 };
    key_map["KP_4"] = { VK_NUMPAD4, 0, 0 };
    key_map["KP_5"] = { VK_NUMPAD5, 0, 0 };
    key_map["KP_6"] = { VK_NUMPAD6, 0, 0 };
    key_map["KP_7"] = { VK_NUMPAD7, 0, 0 };
    key_map["KP_8"] = { VK_NUMPAD8, 0, 0 };
    key_map["KP_9"] = { VK_NUMPAD9, 0, 0 };
    key_map["KP_Decimal"] = { VK_DECIMAL, 0, 0 };
    key_map["KP_Divide"] = { VK_DIVIDE, '/', '/' };
    key_map["KP_Multiply"] = { VK_MULTIPLY, '*', '*' };
    key_map["KP_Subtract"] = { VK_SUBTRACT, '-', '-' };
    key_map["KP_Add"] = { VK_ADD, '+', '+' };
    // key_map["KP_Enter"] = { VK_RETURN, '\r', '\r' }; // Handled by ENHANCED_KEY flag usually

    key_map["KP_Home"] = { VK_HOME, 0, 0 };
    key_map["KP_End"] = { VK_END, 0, 0 };
    // Add other nav keys if necessary

    // Cyrillic 'я' is on 'Z' key (0x5A)
    // 0x044F = я, 0x042F = Я
    key_map["я"] = { 'Z', 0x044F, 0x042F };
}

int main(int argc, char** argv) {
    init_key_map();

    if (argc < 2) {
        std::cerr << "Usage: far2l_tester --key <name> [mods...] [--kitty-flags N]" << std::endl;
        return 1;
    }

    std::string key_name;
    KEY_EVENT_RECORD ev = {};
    ev.bKeyDown = 1;
    ev.wRepeatCount = 1;

    int kitty_flags = 0;
    bool is_num = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--key" && i + 1 < argc) {
            key_name = argv[++i];
        } else if (arg == "--shift") {
            ev.dwControlKeyState |= SHIFT_PRESSED;
        } else if (arg == "--ctrl") {
            // far2l usually checks LEFT or RIGHT, let's set LEFT by default
            ev.dwControlKeyState |= LEFT_CTRL_PRESSED;
        } else if (arg == "--alt") {
            ev.dwControlKeyState |= LEFT_ALT_PRESSED;
        } else if (arg == "--caps") {
            ev.dwControlKeyState |= CAPSLOCK_ON;
        } else if (arg == "--num") {
            ev.dwControlKeyState |= NUMLOCK_ON;
            is_num = true;
        } else if (arg == "--kitty-flags" && i + 1 < argc) {
            kitty_flags = std::stoi(argv[++i]);
        } else if (arg == "--action" && i + 1 < argc) {
            std::string act = argv[++i];
            if (act == "release") ev.bKeyDown = 0;
            // repeat not handled in simple test
        }
    }

    if (key_name.empty()) {
        std::cerr << "Error: --key missing" << std::endl;
        return 1;
    }

    // Special handling for KP_Enter which in Windows is usually VK_RETURN + ENHANCED_KEY
    if (key_name == "KP_Enter") {
        ev.wVirtualKeyCode = VK_RETURN;
        ev.uChar.UnicodeChar = '\r';
        ev.dwControlKeyState |= ENHANCED_KEY;
    } else if (key_map.count(key_name)) {
        KeyDef def = key_map[key_name];
        ev.wVirtualKeyCode = def.vk;

        WORD vk = ev.wVirtualKeyCode;
        if (
            vk == VK_INSERT  || vk == VK_DELETE ||
            vk == VK_HOME    || vk == VK_END    ||
            vk == VK_PRIOR   || vk == VK_NEXT   ||
            vk == VK_UP      || vk == VK_DOWN   ||
            vk == VK_LEFT    || vk == VK_RIGHT  ||
            vk == VK_DIVIDE)
        {
            if (key_name.rfind("KP_", 0) != 0) // keypad keys are definitely not enhanced
                ev.dwControlKeyState |= ENHANCED_KEY;
        }

        bool shift = (ev.dwControlKeyState & SHIFT_PRESSED);
        bool ctrl = (ev.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED));
        bool alt = (ev.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED));

        // Determine char code
        // For simple letters/numbers:
        if (def.ch != 0) {

			bool isLetter = ev.wVirtualKeyCode >= 'A' && ev.wVirtualKeyCode <= 'Z';
			bool useShifted = isLetter ? (shift ^ ((ev.dwControlKeyState & CAPSLOCK_ON) != 0)) : shift;
			ev.uChar.UnicodeChar = useShifted ? def.shift_ch : def.ch;

            // Ctrl behavior: usually transforms char to control code (1-26)
            if (ctrl && !alt && isalpha(def.ch)) {
                ev.uChar.UnicodeChar = (WCHAR)(toupper(def.ch) - 'A' + 1);
            }
        }

        // Numpad logic adjustment
        // If it's a keypad key and NumLock is ON, it produces a digit/char
        // If NumLock is OFF, it acts as navigation (VK_HOME etc), but run_tests passes KP_0...
        // far2l logic handles VK_NUMPADx.
        if (key_name.rfind("KP_", 0) == 0 && key_name.length() == 4 && isdigit(key_name[3])) {
             if (is_num) {
                 // Produce digit
                 ev.uChar.UnicodeChar = key_name[3];
             } else {
                 // Acts as nav key, usually UnicodeChar is 0
                 ev.uChar.UnicodeChar = 0;
                 // Note: VK_NUMPAD0 etc are still the VK codes.
                 // far2l logic might convert them to base keys (VK_INSERT etc) if they were passed that way,
                 // but here we pass VK_NUMPADx and let far2l handle it (or not).
                 // Actually far2l seems to expect VK_INSERT if NumLock is off?
                 // Let's check source logic... logic uses `VK_NUMPAD0` in switch.
                 // It seems far2l expects OS to translate scan codes.
                 // For testing purposes, we stick to VK_NUMPADx unless we want to simulate full driver.
             }
        }

    } else {
        std::cerr << "Error: Unknown key " << key_name << std::endl;
        return 1;
    }

    // Call the function
    std::string result = VT_TranslateKeyToKitty(ev, kitty_flags, 0);

    if (result.empty()) {
        std::cout << "[EMPTY]";
    } else {
        std::cout << result;
    }

    return 0;
}