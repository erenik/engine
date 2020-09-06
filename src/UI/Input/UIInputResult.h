/// Emil Hedemalm
/// 2020-09-05
/// For Input elements, parents reacting to updates.

#pragma once 

enum class UIInputResult {
	NoUpdate,
	InputBegun,
	InputStopped,
	TextUpdated
};
