/// Emil Hedemalm
/// 2015-03-04
/// Handling of it. SpaceShooter2D is getting too long for my taste to contain more...

#include "SideScroller.h"

#include "Input/Action.h"
#include "Input/InputManager.h"

/// Creates default key-bindings for the state.
void SideScroller::CreateDefaultBindings()
{
	List<Binding*> & bindings = this->inputMapping.bindings;
#define BINDING(a,b) bindings.Add(new Binding(a,b));
	BINDING(Action::FromString("ResetCamera"), KEY::HOME);
	BINDING(Action::FromString("StartGame"), List<int>(KEY::N, KEY::G));
	BINDING(Action::FromString("ClearLevel"), List<int>(KEY::C, KEY::L));
	BINDING(Action::FromString("ListEntitiesAndRegistrations"), List<int>(KEY::L, KEY::E));
	BINDING(Action::FromString("ToggleBlackness"), List<int>(KEY::T, KEY::B));
	BINDING(Action::FromString("NextLevel"), List<int>(KEY::N, KEY::L));
	BINDING(Action::FromString("PreviousLevel"), List<int>(KEY::P, KEY::L));
	BINDING(Action::FromString("ToggleMenu"), KEY::ESCAPE);
	BINDING(Action::FromString("ToggleMute"), List<int>(KEY::CTRL, KEY::M));
#define BIND BINDING
	BIND(Action::FromString("AdjustMasterVolume(0.05)", ACTIVATE_ON_REPEAT), List<int>(KEY::CTRL, KEY::V, KEY::PLUS));
	BIND(Action::FromString("AdjustMasterVolume(-0.05)", ACTIVATE_ON_REPEAT), List<int>(KEY::CTRL, KEY::V, KEY::MINUS));
	BIND(Action::FromString("NewGame"), List<int>(KEY::CTRL, KEY::N));
	BIND(Action::FromString("NextK"), List<int>(KEY::CTRL, KEY::RIGHT));
	BIND(Action::FromString("Jump"), KEY::SPACEBAR);
	BIND(Action::FromString("PacoTaco"), List<int>(KEY::CTRL, KEY::P));

	BIND(Action::FromString("RecreateLevelParts"), List<int>(KEY::CTRL, KEY::R, KEY::L));
	BIND(Action::FromString("CycleCamera"), List<int>(KEY::CTRL, KEY::C));

}

