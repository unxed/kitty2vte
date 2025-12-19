#![allow(dead_code)]
#![allow(unused_variables)]

// Mocks for bitflags macro
#[macro_export]
macro_rules! bitflags {
    (
        $(#[$outer:meta])*
        $vis:vis struct $BitFlags:ident: $T:ty {
            $(
                $(#[$inner:ident $($args:tt)*])*
                const $Flag:ident = $value:expr;
            )*
        }
    ) => {
        #[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
        $vis struct $BitFlags($T);

        impl $BitFlags {
            $vis const fn empty() -> Self { Self(0) }
            $vis const fn all() -> Self { Self(!0) }
            $vis const fn bits(&self) -> $T { self.0 }
            $vis const fn from_bits(bits: $T) -> Option<Self> { Some(Self(bits)) }
            $vis const fn from_bits_truncate(bits: $T) -> Self { Self(bits) }
            
            $vis const fn is_empty(&self) -> bool { self.0 == 0 }

            $vis fn contains(&self, other: Self) -> bool {
                (self.0 & other.0) == other.0
            }
            $vis fn intersects(&self, other: Self) -> bool {
                (self.0 & other.0) != 0
            }
            $vis fn insert(&mut self, other: Self) {
                self.0 |= other.0;
            }
            $vis fn remove(&mut self, other: Self) {
                self.0 &= !other.0;
            }
            $vis fn set(&mut self, other: Self, value: bool) {
                if value { self.insert(other); } else { self.remove(other); }
            }
            
            $(
                $vis const $Flag: Self = Self($value);
            )*
        }
        
        impl std::ops::BitOr for $BitFlags {
            type Output = Self;
            fn bitor(self, rhs: Self) -> Self { Self(self.0 | rhs.0) }
        }
        impl std::ops::BitAnd for $BitFlags {
            type Output = Self;
            fn bitand(self, rhs: Self) -> Self { Self(self.0 & rhs.0) }
        }
        impl std::ops::Not for $BitFlags {
            type Output = Self;
            fn not(self) -> Self { Self(!self.0) }
        }
        impl From<$BitFlags> for $T {
            fn from(val: $BitFlags) -> Self { val.0 }
        }
    };
}

// Mocking winit::event::ElementState
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ElementState {
    Pressed,
    Released,
}

impl ElementState {
    pub fn is_pressed(&self) -> bool {
        *self == ElementState::Pressed
    }
}

// Mocking winit::keyboard::KeyLocation
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum KeyLocation {
    Standard,
    Left,
    Right,
    Numpad,
}

// Mocking winit::keyboard::NamedKey
// Added Copy to allow move semantics matching winit behavior
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum NamedKey {
    Enter, Tab, Space, Backspace, Escape,
    ArrowLeft, ArrowRight, ArrowUp, ArrowDown,
    Home, End, PageUp, PageDown, Insert, Delete,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, 
    F25, F26, F27, F28, F29, F30, F31, F32, F33, F34, F35,
    CapsLock, ScrollLock, NumLock, PrintScreen, Pause, ContextMenu,
    Shift, Control, Alt, Super, Hyper, Meta,
    MediaPlay, MediaPause, MediaPlayPause, MediaStop, MediaFastForward,
    MediaRewind, MediaTrackNext, MediaTrackPrevious, MediaRecord,
    AudioVolumeDown, AudioVolumeUp, AudioVolumeMute,
    Unidentified,
}

impl NamedKey {
    pub fn to_text(&self) -> Option<&'static str> {
        match self {
            NamedKey::Enter => Some("\r"),
            NamedKey::Tab => Some("\t"),
            NamedKey::Space => Some(" "),
            NamedKey::Backspace => Some("\x7f"),
            NamedKey::Escape => Some("\x1b"),
            _ => None,
        }
    }
}

// Mocking winit::keyboard::Key
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Key {
    Character(&'static str),
    Named(NamedKey),
    Unidentified(String),
}

impl Key {
    // Allows `key.logical_key.as_ref()` syntax used in Alacritty code
    pub fn as_ref(&self) -> &Self {
        self
    }
}

// Mocking winit::keyboard::ModifiersState
bitflags! {
    pub struct ModifiersState: u32 {
        const SHIFT    = 0b100;
        const CONTROL  = 0b1000;
        const ALT      = 0b10000;
        const SUPER    = 0b100000;
    }
}

impl ModifiersState {
    pub fn shift_key(&self) -> bool { self.contains(Self::SHIFT) }
    pub fn control_key(&self) -> bool { self.contains(Self::CONTROL) }
    pub fn alt_key(&self) -> bool { self.contains(Self::ALT) }
    pub fn super_key(&self) -> bool { self.contains(Self::SUPER) }
}

// Mocking alacritty_terminal::term::TermMode
// Copied values from source/mod.rs to ensure compatibility
bitflags! {
    pub struct TermMode: u32 {
        const NONE                    = 0;
        const SHOW_CURSOR             = 1;
        const APP_CURSOR              = 1 << 1;
        const APP_KEYPAD              = 1 << 2;
        const MOUSE_REPORT_CLICK      = 1 << 3;
        const BRACKETED_PASTE         = 1 << 4;
        const SGR_MOUSE               = 1 << 5;
        const MOUSE_MOTION            = 1 << 6;
        const LINE_WRAP               = 1 << 7;
        const LINE_FEED_NEW_LINE      = 1 << 8;
        const ORIGIN                  = 1 << 9;
        const INSERT                  = 1 << 10;
        const FOCUS_IN_OUT            = 1 << 11;
        const ALT_SCREEN              = 1 << 12;
        const MOUSE_DRAG              = 1 << 13;
        const UTF8_MOUSE              = 1 << 14;
        const ALTERNATE_SCROLL        = 1 << 15;
        const VI                      = 1 << 16;
        const URGENCY_HINTS           = 1 << 17;
        const DISAMBIGUATE_ESC_CODES  = 1 << 18;
        const REPORT_EVENT_TYPES      = 1 << 19;
        const REPORT_ALTERNATE_KEYS   = 1 << 20;
        const REPORT_ALL_KEYS_AS_ESC  = 1 << 21;
        const REPORT_ASSOCIATED_TEXT  = 1 << 22;
    }
}

// Mocking winit::event::KeyEvent
#[derive(Debug, Clone)]
pub struct KeyEvent {
    pub logical_key: Key,
    pub location: KeyLocation,
    pub state: ElementState,
    pub repeat: bool,
    
    // Test helper to store pre-calculated text
    pub test_text: Option<&'static str>,
}

impl KeyEvent {
    pub fn text_with_all_modifiers(&self) -> Option<&'static str> {
        self.test_text
    }
    
    // In alacritty code, this is used to check for "unmodded" key.
    // We mock it simply for the test cases.
    pub fn key_without_modifiers(&self) -> Key {
        // Simplified: return the key itself as if unmodded. 
        // Logic in try_build_textual handles char extraction.
        self.logical_key.clone()
    }
}