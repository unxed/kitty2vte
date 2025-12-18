#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "vte_key_tester.h"

std::map<std::string, guint> keyval_map;

void initialize_keyval_map() {
    // Letters
    for (char c = 'a'; c <= 'z'; ++c) {
        keyval_map[std::string(1, c)] = GDK_KEY_a + (c - 'a');
    }
    // Numbers
    for (char c = '0'; c <= '9'; ++c) {
        keyval_map[std::string(1, c)] = GDK_KEY_0 + (c - '0');
    }
    // Function keys
    for (int i = 1; i <= 12; ++i) {
        keyval_map["F" + std::to_string(i)] = GDK_KEY_F1 + (i - 1);
    }
    // Other keys
    keyval_map["minus"] = GDK_KEY_minus;
    keyval_map["equal"] = GDK_KEY_equal;
    keyval_map["space"] = GDK_KEY_space;
    keyval_map["Escape"] = GDK_KEY_Escape;
    keyval_map["Tab"] = GDK_KEY_Tab;
    keyval_map["Return"] = GDK_KEY_Return;
    keyval_map["BackSpace"] = GDK_KEY_BackSpace;
    keyval_map["Home"] = GDK_KEY_Home;
    keyval_map["End"] = GDK_KEY_End;
    keyval_map["Up"] = GDK_KEY_Up;
    keyval_map["Down"] = GDK_KEY_Down;
    keyval_map["Left"] = GDK_KEY_Left;
    keyval_map["Right"] = GDK_KEY_Right;
    keyval_map["Insert"] = GDK_KEY_Insert;
    keyval_map["Delete"] = GDK_KEY_Delete;
    keyval_map["Page_Up"] = GDK_KEY_Page_Up;
    keyval_map["Page_Down"] = GDK_KEY_Page_Down;
    keyval_map["bracketleft"] = GDK_KEY_bracketleft;
    keyval_map["bracketright"] = GDK_KEY_bracketright;
    keyval_map["backslash"] = GDK_KEY_backslash;
    keyval_map["semicolon"] = GDK_KEY_semicolon;
    keyval_map["apostrophe"] = GDK_KEY_apostrophe;
    keyval_map["comma"] = GDK_KEY_comma;
    keyval_map["period"] = GDK_KEY_period;
    keyval_map["slash"] = GDK_KEY_slash;
    keyval_map["`"] = GDK_KEY_grave;

    // Shifted keys mapped by name
    keyval_map["~"] = GDK_KEY_asciitilde;
    keyval_map["_"] = GDK_KEY_underscore;
    keyval_map["+"] = GDK_KEY_plus;
    keyval_map["{"] = GDK_KEY_braceleft;
    keyval_map["}"] = GDK_KEY_braceright;
    keyval_map["|"] = GDK_KEY_bar;
    keyval_map[":"] = GDK_KEY_colon;
    keyval_map["\""] = GDK_KEY_quotedbl;
    keyval_map["<"] = GDK_KEY_less;
    keyval_map[">"] = GDK_KEY_greater;
    keyval_map["?"] = GDK_KEY_question;

    // Keypad keys
    for(int i = 0; i <= 9; ++i) {
        keyval_map["KP_" + std::to_string(i)] = GDK_KEY_KP_0 + i;
    }
    keyval_map["KP_Home"] = GDK_KEY_KP_Home;
    keyval_map["KP_End"] = GDK_KEY_KP_End;
}

int main(int argc, char** argv) {
    initialize_keyval_map();
    
    if (argc < 4) {
        std::cerr << "Usage: ./tester --key <Key_Name> --keycode <num> [--shift] [--ctrl] [--alt] [--kitty-flags <num>] [--action <press|release|repeat>]" << std::endl;
        return 1;
    }

    std::string key_name;
    guint keycode = 0;
    GdkModifierType modifiers = (GdkModifierType)0;
    int kitty_flags = 0;
    bool is_press = true;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--key" && i + 1 < argc) key_name = argv[++i];
        else if (arg == "--keycode" && i + 1 < argc) keycode = std::stoi(argv[++i]);
        else if (arg == "--shift") modifiers = (GdkModifierType)(modifiers | GDK_SHIFT_MASK);
        else if (arg == "--ctrl") modifiers = (GdkModifierType)(modifiers | GDK_CONTROL_MASK);
        else if (arg == "--alt") modifiers = (GdkModifierType)(modifiers | VTE_ALT_MASK);
        else if (arg == "--caps") modifiers = (GdkModifierType)(modifiers | GDK_LOCK_MASK);
        else if (arg == "--num") modifiers = (GdkModifierType)(modifiers | VTE_NUMLOCK_MASK);
        else if (arg == "--kitty-flags" && i + 1 < argc) kitty_flags = std::stoi(argv[++i]);
        else if (arg == "--action" && i + 1 < argc) {
            std::string action_str = argv[++i];
            if (action_str == "release") is_press = false;
        }
    }

    if (key_name.empty() || keycode == 0) {
        std::cerr << "Error: --key and --keycode are required." << std::endl;
        return 1;
    }

    guint keyval = 0;
    if (keyval_map.count(key_name)) {
        keyval = keyval_map[key_name];
    } else {
        std::cerr << "Error: Unknown key name '" << key_name << "'" << std::endl;
        return 1;
    }

    TesterTerminal terminal;
    terminal.set_kitty_keyboard_flags(kitty_flags);

    MockKeyEvent event(keyval, keycode, modifiers, is_press);
    
    terminal.widget_key_press(event);

    return 0;
}