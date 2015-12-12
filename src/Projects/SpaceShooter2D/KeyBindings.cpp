/// Emil Hedemalm
/// 2015-03-04
/// Handling of it. SpaceShooter2D is getting too long for my taste to contain more...

#include "SpaceShooter2D/SpaceShooter2D.h"

#include "Input/Action.h"
#include "Input/InputManager.h"

/// Creates default key-bindings for the state.
void SpaceShooter2D::CreateDefaultBindings()
{
	List<Binding*> & bindings = this->inputMapping.bindings;
#define BINDING(a,b) bindings.Add(new Binding(a,b));
	BINDING(Action::FromString("TogglePlayerInvulnerability"), KEY::I);
	BINDING(Action::CreateStartStopAction("MoveShipUp"), KEY::W);
	BINDING(Action::CreateStartStopAction("MoveShipDown"), KEY::S);
	BINDING(Action::CreateStartStopAction("MoveShipLeft"), KEY::A);
	BINDING(Action::CreateStartStopAction("MoveShipRight"), KEY::D);
	BINDING(Action::FromString("ToggleWeaponScript"), KEY::E);
	BINDING(Action::FromString("ActivateSkill"), KEY::Q);
	BINDING(Action::FromString("ResetCamera"), KEY::HOME);
	BINDING(Action::FromString("NewGame"), List<int>(KEY::N, KEY::G));
	BINDING(Action::FromString("ClearLevel"), List<int>(KEY::C, KEY::L));
	BINDING(Action::FromString("ListEntitiesAndRegistrations"), List<int>(KEY::L, KEY::E));
	BINDING(Action::FromString("ToggleBlackness"), List<int>(KEY::T, KEY::B));
	BINDING(Action::FromString("NextLevel"), List<int>(KEY::N, KEY::L));
	BINDING(Action::FromString("PreviousLevel"), List<int>(KEY::P, KEY::L));
	BINDING(Action::FromString("ToggleMenu"), KEY::ESCAPE);
	BINDING(Action::FromString("ToggleMute"), KEY::M);
#define BIND BINDING
	BIND(Action::FromString("AdjustMasterVolume(0.05)", ACTIVATE_ON_REPEAT), List<int>(KEY::CTRL, KEY::V, KEY::PLUS));
	BIND(Action::FromString("AdjustMasterVolume(-0.05)", ACTIVATE_ON_REPEAT), List<int>(KEY::CTRL, KEY::V, KEY::MINUS));

	for (int i = 0; i < WeaponType::MAX_TYPES; ++i)
	{
		/// Debug bindings for adjusting weapon levels mid-flight.
		BIND(Action::FromString("IncreaseWeaponLevel:"+String(i)), List<int>(KEY::PLUS, KEY::ONE+i));
		BIND(Action::FromString("DecreaseWeaponLevel:"+String(i)), List<int>(KEY::MINUS, KEY::ONE+i));
		BIND(Action::FromString("IncreaseWeaponLevel:"+String(i)), List<int>(KEY::ONE+i, KEY::PLUS));
		BIND(Action::FromString("DecreaseWeaponLevel:"+String(i)), List<int>(KEY::ONE+i, KEY::MINUS));
		BIND(Action::FromString("Weapon:"+String(i+1)), KEY::ONE + i);
	}

	BIND(Action::FromString("ListPhysicalEntities"), List<int>(KEY::L, KEY::P));
	BIND(Action::FromString("Reload OnEnter"), List<int>(KEY::CTRL, KEY::O, KEY::E));
	/*
	BIND(Action::FromString("Weapon:1"), KEY::ONE);
	BIND(Action::FromString("Weapon:2"), KEY::TWO);
	BIND(Action::FromString("Weapon:3"), KEY::THREE);
	BIND(Action::FromString("Weapon:4"), KEY::FOUR);
	BIND(Action::FromString("Weapon:5"), KEY::FIVE);
	*/
	BIND(Action::CreateStartStopAction("Shooting"), KEY::SPACE);
	BIND(Action::FromString("OpenJumpDialog"), List<int>(KEY::CTRL, KEY::G));
	BIND(Action::FromString("ProceedMessage"), KEY::ENTER);

}

