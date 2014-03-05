// Emil Hedemalm
// 2013-07-11

#include "Ship.h"
#include "Entity/Entity.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Texture.h"

Ship::Ship(){
	// Gameplay stats
	// Local Z-wise acceleration.
	thrust = 0.00035f;
	reverse = 0.00004f;
	// For turning, local angular acceleration.
	angularThrust = 0.0002f;


	// For boosting the Z-wise acceleration.
	boostMultiplier = 1.5f; // Boost multiplier
	maxBoost = 5.0f;
	boostRegeneration = 0.1f;

	// Survival
	maxHP = 500.0f;
	hpRegeneration = 1.0f;

	modelSource = "racing/ship_UVd.obj";
	diffuseSource = "racing/ship_textured.png";

	// Physics stuff
	velocityRetainedWhileTurning = 0.6f;

	thrusterPosition = Vector3f(0,0,5);
};


/// Sends GMSetTexture messages to the entity, and maybe more parameters later.
void Ship::AssignTexturesToEntity(Entity * entity){
    Graphics.QueueMessage(new GMSetEntityTexture(entity, DIFFUSE_MAP, diffuseSource));
    Graphics.QueueMessage(new GMSetEntityTexture(entity, SPECULAR_MAP, specularSource));
    Graphics.QueueMessage(new GMSetEntityTexture(entity, NORMAL_MAP, normalSource));
}

#define Abs(a) AbsoluteValue(a)

/// Makes sure it's stats are valid. Returns false if it does not.
bool Ship::HasValidStats(){
	if (Abs(thrust) <= 0){
		std::cout<<"\nInvalid thrust";
		return false;
	}
	if (Abs(angularThrust) <= 0){
		std::cout<<"\nInvalid angularThrust";
		return false;
	}
	return true;
}