/// Just an enumeration over all available keyboard-keys ^^

#include "Keys.h"


String keyStrings[KEY::TOTAL_KEYS];



bool initialized = false;

void InitializeKeyStrings(){
    for (int i = 0; i < KEY::TOTAL_KEYS; ++i){
        if (i >= KEY::A && i <= KEY::Z)
            keyStrings[i] = (char) ('A' + i - KEY::A);
        else if (i >= KEY::ZERO && i <= KEY::NINE)
            keyStrings[i] = (char) ('0' + i - KEY::ZERO);
        else if (i >= KEY::F1 && i <= KEY::F12)
            keyStrings[i] = "F" + String::ToString(i - KEY::F1 + 1);
        else
            keyStrings[i] = "Lazy programmer-key.";
    }
    keyStrings[0] = "NULL-key";
    keyStrings[KEY::ENTER] = "Enter";
    keyStrings[KEY::CTRL] = "CTRL";
    keyStrings[KEY::SHIFT] = "Shift";
    keyStrings[KEY::ALT] = "Alt";
    keyStrings[KEY::LEFT] = "Left";
    keyStrings[KEY::RIGHT] = "Right";
    keyStrings[KEY::UP] = "Up";
    keyStrings[KEY::DOWN] = "Down";
    keyStrings[KEY::HOME] = "Home";
    keyStrings[KEY::PG_UP] = "Page up";
    keyStrings[KEY::PG_DOWN] = "Page down";
    keyStrings[KEY::COMMA] = "Comma";
    keyStrings[KEY::DECIMAL] = "Decimal";
    keyStrings[KEY::ESCAPE] = "Escape";
    initialized = true;

	for (int i = 0; i < KEY::TOTAL_KEYS; ++i)
	{
		std::cout<<"\nKeyString "<<i<<": "<<keyStrings[i];
	}
}


/// For displaying ^^
String GetKeyString(int keyCode){
    if (!initialized)
        InitializeKeyStrings();
    return keyStrings[keyCode];
}

/*
namespace KEY {
	enum KEY{
		NULL_KEY,
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		a = A, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
		ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE,
		CTRL, SHIFT, ALT, ALT_GR, ESCAPE, ESC = ESCAPE, SPACE, SPACEBAR = SPACE,
		ENTER, BACKSPACE, DELETE_KEY, TAB,
		LEFT, RIGHT, UP, DOWN,
		HOME, PG_UP, PG_DOWN, END,
		PAUSE_BREAK, SCROLL_LOCK, CAPS_LOCK, INSERT, PRINT_SCREEN,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		SECTION, /* §
		COMMA, PUNCTUATION, PERIOD = PUNCTUATION, DASH, MINUS = DASH, PLUS, /* , . - +
		APOSTROPHE, /* ´
		MENU, /* that ... button... ;__;
		TOTAL_KEYS,
	};
};
*/

