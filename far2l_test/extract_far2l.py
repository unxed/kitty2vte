#!/usr/bin/env python3
import sys
import os
from pathlib import Path

def extract_function_body(source_path, dest_path):
    if not os.path.exists(source_path):
        print(f"Error: Source file '{source_path}' not found.", file=sys.stderr)
        sys.exit(1)

    with open(source_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    body = []
    in_function = False
    brace_level = 0
    
    # Signature of the function we want to extract
    func_sig = "std::string VT_TranslateKeyToKitty"

    for line in lines:
        if not in_function:
            if func_sig in line and "{" in line:
                in_function = True
                brace_level = line.count("{") - line.count("}")
                body.append("// Extracted body of VT_TranslateKeyToKitty\n")
            elif func_sig in line:
                in_function = True
                brace_level = 0
        else:
            # We are inside the function
            # If we just started and brace_level is 0, wait for opening brace
            if brace_level == 0 and "{" in line:
                brace_level += line.count("{")
                brace_level -= line.count("}")
                continue # Skip the line with the first opening brace if it was separate

            brace_level += line.count("{")
            brace_level -= line.count("}")

            if brace_level <= 0:
                break
            
            body.append(line)

    if not body:
        print(f"Error: Function body for '{func_sig}' not found or empty.", file=sys.stderr)
        sys.exit(1)

    with open(dest_path, 'w', encoding='utf-8') as f:
        f.writelines(body)


    # As far2l's API does not allow us to get unshifted and shifted key values
    # (we only have UnicodeChar value with shift/caps already applied),
    # let's emulate shifted/unshifted behavior for US keyboard layout we use for testing.
    # Errors that cannot be corrected anyway will be excluded from the test
    # but they can hardly be considered critical.

    FILE = Path("far2l_test/far2l_key_press_body.inc")

    HEADER = r'''
/* === AUTO-GENERATED SHIFTED / UNSHIFTED SUPPORT FOR TESTING === */

auto vkToEnChars = [](int vk) -> std::pair<char, char> {
    // A–Z
    if (vk >= 'A' && vk <= 'Z')
        return { char(vk + 32), char(vk) }; // a / A

    // 0–9
    switch (vk) {
        case '0': return {'0', ')'};
        case '1': return {'1', '!'};
        case '2': return {'2', '@'};
        case '3': return {'3', '#'};
        case '4': return {'4', '$'};
        case '5': return {'5', '%'};
        case '6': return {'6', '^'};
        case '7': return {'7', '&'};
        case '8': return {'8', '*'};
        case '9': return {'9', '('};

        case VK_OEM_MINUS:      return {'-', '_'};
        case VK_OEM_PLUS:       return {'=', '+'};
        case VK_OEM_4:          return {'[', '{'};
        case VK_OEM_6:          return {']', '}'};
        case VK_OEM_5:          return {'\\','|'};
        case VK_OEM_1:          return {';', ':'};
        case VK_OEM_7:          return {'\'','"'};
        case VK_OEM_COMMA:      return {',', '<'};
        case VK_OEM_PERIOD:     return {'.', '>'};
        case VK_OEM_2:          return {'/', '?'};
        case VK_OEM_3:          return {'`', '~'};
        case VK_SPACE:          return {' ', ' '};
    }
    return {0, 0}; // не печатающая клавиша
};

/* === END AUTO-GENERATED === */

'''

    REPL1 = r'''
	auto [unshifted_src, shifted_src] = vkToEnChars(KeyEvent.wVirtualKeyCode);
    keycode = unshifted_src;
    '''.strip()

    REPL2 = r'''
	auto [unshifted_src, shifted_src] = vkToEnChars(KeyEvent.wVirtualKeyCode);
    shifted = shifted_src;
    '''.strip()


    text = FILE.read_text(encoding="utf-8")

    if "AUTO-GENERATED SHIFT / UNSHIFT SUPPORT" not in text:
        text = HEADER + "\n" + text

    text = text.replace(
        "keycode = towlower(KeyEvent.uChar.UnicodeChar);",
        REPL1
    )

    text = text.replace(
        "shifted = KeyEvent.uChar.UnicodeChar;",
        REPL2
    )

    FILE.write_text(text, encoding="utf-8")
    print("OK: файл обработан")


    print(f"[*] Extracted far2l logic to '{dest_path}'")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        # Default behavior for Makefile simplicity if called without args, 
        # though Makefile should ideally pass them.
        source = 'source/vtshell_translation_kitty.cpp'
        dest = 'far2l_test/far2l_key_press_body.inc'
    else:
        source = sys.argv[1]
        dest = 'far2l_test/far2l_key_press_body.inc'

    extract_function_body(source, dest)