/// Just an enumeration over all available keyboard-keys ^^

#ifndef KEYS_H
#define KEYS_H

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
		SECTION, /* § */
		COMMA, PUNCTUATION, PERIOD = PUNCTUATION, DECIMAL = PERIOD, DASH, MINUS = DASH, PLUS, /* , . - + */
		APOSTROPHE, /* ´ */
		MENU, /* that ... button... ;__; */
		TOTAL_KEYS,
	};
};

#include "String/AEString.h"

/// For displaying ^^
String GetKeyString(int keyCode);

#endif
