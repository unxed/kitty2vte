mod alacritty_mocks;
use alacritty_mocks::*;

// Include the extracted logic
include!("alacritty_extracted.rs");

use std::env;
use std::io::{self, Write};

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() < 2 {
        eprintln!("Usage: alacritty_tester --key <name> [--shift] [--ctrl] [--alt] [--super] [--caps] [--num] [--kitty-flags N] [--action <press|release|repeat>]");
        return;
    }

    let mut key_name = String::new();
    let mut mods = ModifiersState::empty();
    let mut kitty_flags = 0u32;
    let mut action = ElementState::Pressed;
    let mut repeat = false;
    let mut caps = false;
    let mut num = false;

    let mut i = 1;
    while i < args.len() {
        match args[i].as_str() {
            "--key" => {
                if i + 1 < args.len() {
                    key_name = args[i+1].clone();
                    i += 1;
                }
            },
            "--shift" => mods.insert(ModifiersState::SHIFT),
            "--ctrl" => mods.insert(ModifiersState::CONTROL),
            "--alt" => mods.insert(ModifiersState::ALT),
            "--super" => mods.insert(ModifiersState::SUPER),
            "--caps" => caps = true,
            "--num" => num = true,
            "--kitty-flags" => {
                if i + 1 < args.len() {
                    kitty_flags = args[i+1].parse().unwrap_or(0);
                    i += 1;
                }
            },
            "--action" => {
                if i + 1 < args.len() {
                    match args[i+1].as_str() {
                        "release" => action = ElementState::Released,
                        "repeat" => { action = ElementState::Pressed; repeat = true; },
                        _ => action = ElementState::Pressed,
                    }
                    i += 1;
                }
            },
            _ => {}
        }
        i += 1;
    }

    let mut mode = TermMode::from_bits_truncate(0); // Start empty
    // Map kitty protocol flags (1, 2, 4, 8, 16) to TermMode bits
    if (kitty_flags & 1) != 0 { mode.insert(TermMode::DISAMBIGUATE_ESC_CODES); }
    if (kitty_flags & 2) != 0 { mode.insert(TermMode::REPORT_EVENT_TYPES); }
    if (kitty_flags & 4) != 0 { mode.insert(TermMode::REPORT_ALTERNATE_KEYS); }
    if (kitty_flags & 8) != 0 { mode.insert(TermMode::REPORT_ALL_KEYS_AS_ESC); }
    if (kitty_flags & 16) != 0 { mode.insert(TermMode::REPORT_ASSOCIATED_TEXT); }

    let (logical_key, location, text_val) = map_key_name(&key_name, mods, caps, num);

    let key_event = KeyEvent {
        logical_key,
        location,
        state: action,
        repeat,
        test_text: text_val,
    };

    let text_str = key_event.text_with_all_modifiers().unwrap_or("");

    // Simulate the logic in Alacritty's Processor::key_input
    let should_build = should_build_sequence(&key_event, text_str, mode, mods);

    if should_build {
        let result = build_sequence(key_event, mods, mode);
        if result.is_empty() {
            print!("[EMPTY]");
        } else {
            io::stdout().write_all(&result).unwrap();
        }
    } else {
        // If we shouldn't build a sequence, Alacritty emits the text directly
        if !text_str.is_empty() {
             print!("{}", text_str);
        } else {
             print!("[EMPTY]");
        }
    }
}

fn map_key_name(name: &str, mods: ModifiersState, caps: bool, num: bool) -> (Key, KeyLocation, Option<&'static str>) {
    let shift = mods.contains(ModifiersState::SHIFT);

    // Logic:
    // 1. If it is a single letter (a-z), CapsLock inverts Shift.
    // 2. If it is a digit or symbol, CapsLock usually does nothing (standard US layout), only Shift matters.
    // 3. For Keypad, NumLock matters (handled separately below).

    // Helper for char keys
    let char_key = |c: &'static str, shift_c: &'static str| {
        let first_char = c.chars().next().unwrap();
        let is_letter = first_char.is_ascii_alphabetic();

        let use_shifted = if is_letter {
            shift ^ caps
        } else {
            shift
        };

        let text = if use_shifted { shift_c } else { c };
        (Key::Character(text), KeyLocation::Standard, Some(text))
    };

    match name {
        // Function Keys
        "F1" => (Key::Named(NamedKey::F1), KeyLocation::Standard, None),
        "F2" => (Key::Named(NamedKey::F2), KeyLocation::Standard, None),
        "F3" => (Key::Named(NamedKey::F3), KeyLocation::Standard, None),
        "F4" => (Key::Named(NamedKey::F4), KeyLocation::Standard, None),
        "F5" => (Key::Named(NamedKey::F5), KeyLocation::Standard, None),
        "F6" => (Key::Named(NamedKey::F6), KeyLocation::Standard, None),
        "F7" => (Key::Named(NamedKey::F7), KeyLocation::Standard, None),
        "F8" => (Key::Named(NamedKey::F8), KeyLocation::Standard, None),
        "F9" => (Key::Named(NamedKey::F9), KeyLocation::Standard, None),
        "F10" => (Key::Named(NamedKey::F10), KeyLocation::Standard, None),
        "F11" => (Key::Named(NamedKey::F11), KeyLocation::Standard, None),
        "F12" => (Key::Named(NamedKey::F12), KeyLocation::Standard, None),

        // Control
        "Escape" => (Key::Named(NamedKey::Escape), KeyLocation::Standard, None),
        "Return" => (Key::Named(NamedKey::Enter), KeyLocation::Standard, Some("\r")),
        "Tab" => (Key::Named(NamedKey::Tab), KeyLocation::Standard, Some("\t")),
        "BackSpace" => (Key::Named(NamedKey::Backspace), KeyLocation::Standard, Some("\x7f")),
        "space" => (Key::Named(NamedKey::Space), KeyLocation::Standard, Some(" ")),

        // Navigation
        "Insert" => (Key::Named(NamedKey::Insert), KeyLocation::Standard, None),
        "Delete" => (Key::Named(NamedKey::Delete), KeyLocation::Standard, None),
        "Home" => (Key::Named(NamedKey::Home), KeyLocation::Standard, None),
        "End" => (Key::Named(NamedKey::End), KeyLocation::Standard, None),
        "Page_Up" => (Key::Named(NamedKey::PageUp), KeyLocation::Standard, None),
        "Page_Down" => (Key::Named(NamedKey::PageDown), KeyLocation::Standard, None),
        "Up" => (Key::Named(NamedKey::ArrowUp), KeyLocation::Standard, None),
        "Down" => (Key::Named(NamedKey::ArrowDown), KeyLocation::Standard, None),
        "Left" => (Key::Named(NamedKey::ArrowLeft), KeyLocation::Standard, None),
        "Right" => (Key::Named(NamedKey::ArrowRight), KeyLocation::Standard, None),

        // Keypad
        // If NumLock is ON, digits return numbers.
        // If NumLock is OFF, digits return Navigation keys (handled by winit usually sending ArrowUp etc instead of KP_8).
        // However, here we simulate input. If run_tests sends "KP_0", we assume the physical key.
        // Alacritty logic in `try_build_numpad` checks `key.location == Numpad`.

        // Note: For this tester, we stick to the Python map logic. If run_tests says "KP_0",
        // we construct a Key::Character("0") if numlock is on, or Key::Named(Arrow...) if off?
        // Let's simplify: Alacritty's `try_build_numpad` matches specific logical keys.
        // If we want to simulate NumLock OFF behavior generating escape sequences for Home/Up,
        // we should pass Key::Named(Home) with Location::Numpad.

        "KP_0" => if num { (Key::Character("0"), KeyLocation::Numpad, Some("0")) } else { (Key::Named(NamedKey::Insert), KeyLocation::Numpad, None) },
        "KP_1" => if num { (Key::Character("1"), KeyLocation::Numpad, Some("1")) } else { (Key::Named(NamedKey::End), KeyLocation::Numpad, None) },
        "KP_2" => if num { (Key::Character("2"), KeyLocation::Numpad, Some("2")) } else { (Key::Named(NamedKey::ArrowDown), KeyLocation::Numpad, None) },
        "KP_3" => if num { (Key::Character("3"), KeyLocation::Numpad, Some("3")) } else { (Key::Named(NamedKey::PageDown), KeyLocation::Numpad, None) },
        "KP_4" => if num { (Key::Character("4"), KeyLocation::Numpad, Some("4")) } else { (Key::Named(NamedKey::ArrowLeft), KeyLocation::Numpad, None) },
        "KP_5" => if num { (Key::Character("5"), KeyLocation::Numpad, Some("5")) } else { (Key::Character("5"), KeyLocation::Numpad, None) }, // 5 usually does nothing or is Begin
        "KP_6" => if num { (Key::Character("6"), KeyLocation::Numpad, Some("6")) } else { (Key::Named(NamedKey::ArrowRight), KeyLocation::Numpad, None) },
        "KP_7" => if num { (Key::Character("7"), KeyLocation::Numpad, Some("7")) } else { (Key::Named(NamedKey::Home), KeyLocation::Numpad, None) },
        "KP_8" => if num { (Key::Character("8"), KeyLocation::Numpad, Some("8")) } else { (Key::Named(NamedKey::ArrowUp), KeyLocation::Numpad, None) },
        "KP_9" => if num { (Key::Character("9"), KeyLocation::Numpad, Some("9")) } else { (Key::Named(NamedKey::PageUp), KeyLocation::Numpad, None) },

        "KP_Decimal" => if num { (Key::Character("."), KeyLocation::Numpad, Some(".")) } else { (Key::Named(NamedKey::Delete), KeyLocation::Numpad, None) },
        "KP_Divide" => (Key::Character("/"), KeyLocation::Numpad, Some("/")),
        "KP_Multiply" => (Key::Character("*"), KeyLocation::Numpad, Some("*")),
        "KP_Subtract" => (Key::Character("-"), KeyLocation::Numpad, Some("-")),
        "KP_Add" => (Key::Character("+"), KeyLocation::Numpad, Some("+")),
        "KP_Enter" => (Key::Named(NamedKey::Enter), KeyLocation::Numpad, Some("\r")),
        "KP_Equal" => (Key::Character("="), KeyLocation::Numpad, Some("=")),

        // These keys in run_tests.py (KP_Home, KP_Up) usually imply NumLock is OFF or explicit nav key on keypad
        "KP_Home" => (Key::Named(NamedKey::Home), KeyLocation::Numpad, None),
        "KP_End" => (Key::Named(NamedKey::End), KeyLocation::Numpad, None),
        "KP_Page_Up" => (Key::Named(NamedKey::PageUp), KeyLocation::Numpad, None),
        "KP_Page_Down" => (Key::Named(NamedKey::PageDown), KeyLocation::Numpad, None),
        "KP_Up" => (Key::Named(NamedKey::ArrowUp), KeyLocation::Numpad, None),
        "KP_Down" => (Key::Named(NamedKey::ArrowDown), KeyLocation::Numpad, None),
        "KP_Left" => (Key::Named(NamedKey::ArrowLeft), KeyLocation::Numpad, None),
        "KP_Right" => (Key::Named(NamedKey::ArrowRight), KeyLocation::Numpad, None),
        "KP_Begin" => (Key::Character("5"), KeyLocation::Numpad, None),
        "KP_Insert" => (Key::Named(NamedKey::Insert), KeyLocation::Numpad, None),
        "KP_Delete" => (Key::Named(NamedKey::Delete), KeyLocation::Numpad, None),

        // Characters
        "a" => char_key("a", "A"), "b" => char_key("b", "B"), "c" => char_key("c", "C"),
        "d" => char_key("d", "D"), "e" => char_key("e", "E"), "f" => char_key("f", "F"),
        "g" => char_key("g", "G"), "h" => char_key("h", "H"), "i" => char_key("i", "I"),
        "j" => char_key("j", "J"), "k" => char_key("k", "K"), "l" => char_key("l", "L"),
        "m" => char_key("m", "M"), "n" => char_key("n", "N"), "o" => char_key("o", "O"),
        "p" => char_key("p", "P"), "q" => char_key("q", "Q"), "r" => char_key("r", "R"),
        "s" => char_key("s", "S"), "t" => char_key("t", "T"), "u" => char_key("u", "U"),
        "v" => char_key("v", "V"), "w" => char_key("w", "W"), "x" => char_key("x", "X"),
        "y" => char_key("y", "Y"), "z" => char_key("z", "Z"),

        "1" => char_key("1", "!"), "2" => char_key("2", "@"), "3" => char_key("3", "#"),
        "4" => char_key("4", "$"), "5" => char_key("5", "%"), "6" => char_key("6", "^"),
        "7" => char_key("7", "&"), "8" => char_key("8", "*"), "9" => char_key("9", "("),
        "0" => char_key("0", ")"),

        "`" => char_key("`", "~"), "-" | "minus" => char_key("-", "_"),
        "=" | "equal" => char_key("=", "+"),
        "[" | "bracketleft" => char_key("[", "{"), "]" | "bracketright" => char_key("]", "}"),
        "\\" | "backslash" => char_key("\\", "|"), ";" | "semicolon" => char_key(";", ":"),
        "'" | "apostrophe" => char_key("'", "\""), "," | "comma" => char_key(",", "<"),
        "." | "period" => char_key(".", ">"), "/" | "slash" => char_key("/", "?"),

        "я" => char_key("я", "Я"),

        _ => (Key::Unidentified(name.to_string()), KeyLocation::Standard, None),
    }
}