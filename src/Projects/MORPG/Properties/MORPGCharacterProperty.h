/// Emil Hedemalm
/// 2014-08-01
/// Binds the Entity- and custom MORPG character objects together nicely.

#include "Entity/EntityProperty.h"
#include "MathLib.h"

class Character;

class MORPGCharacterProperty : public EntityProperty 
{
public:
	MORPGCharacterProperty(Entity * characterEntity, Character * associatedWithCharacter);

	/// Time passed in seconds..! Will steer if inputFocus is true.
	virtual void Process(int timeInMs);

	void ToggleAutorun();

	// Default false. Enable to steer this entity.
	bool inputFocus;

private:
	/** Checks states via InputManager. Regular key-bindings should probably still be defined in the main game state 
		and then passed on as messages to the character with inputFocus turned on.
	*/
	void ProcessInput(); 

	Character * ch;

	/// For handling movement.
	Vector3f lastAcc;
	float lastRight;
	bool autorun;
	float movementSpeed;
};




