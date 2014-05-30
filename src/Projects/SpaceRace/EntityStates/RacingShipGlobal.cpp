// Emil Hedemalm
// 2013-06-15

#include "Message/Message.h"
#include "Physics/Messages/CollissionCallback.h"
#include "RacingShipGlobal.h"
#include "EntityStates/EntityStates.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetGraphicEffect.h"
#include "Graphics/GraphicsProperty.h"
#include "Graphics/Effects/GraphicEffect.h"
#include "Graphics/Effects/AlphaModelEffect.h"
#include "../Ship.h"
#include "Physics/PhysicsManager.h"
#include "Graphics/Particles/Exhaust.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"
#include "../SRPlayer.h"
#include "../GameStates/Racing/RacingState.h"
#include "AI/AI.h"
#include "../AI/AIRacer.h"
#include "Player/PlayerManager.h"
#include "Graphics/Messages/GraphicsMessages.h"
#include "String/StringUtil.h"

RacingShipGlobal::RacingShipGlobal(Entity * entity, Ship * ship)
: EntityState(EntityStateType::RACING_SHIP, entity)
{
	/// Hope that the position vector is the starting position..
	startingPosition = entity->position;
	startingRotation = entity->rotation;

	assert(ship && entity);
	std::cout<<"\nRacingShipGlobal constructor.";
	name = entity->name +"'s RacingShipGlobal state";
	shipType = ship;
	std::cout<<"\nAttaching ship with thrust: "<<ship->thrust;
	if (AbsoluteValue(shipType->thrust) <= 0){
		std::cout<<"\nERROR: Ship thrust 0 :(";
		assert(false);
	}
	exhaust = NULL;
	/// States
	thrusting = reversing = left = right = boosting = false;
	lastResetTime = 0;
	resetCooldown = 1000;
	/// Default, no assigned ship.
	if (ship == NULL)
	{
		boostRemaining = boostMax = boostRegen = 0;
	}
	/// New, assigned ship!
	else {
		boostRemaining = ship->maxBoost;
		boostMax = ship->maxBoost;
		boostRegen = ship->boostRegeneration;
	}

	/// To accomodate for better AI and controls like gamepads or joysticks! :)
	relativeThrust = relativeTurn = 0;
	useRelativeValues = false;
	std::cout<<"\nRacingShipGlobal constructor done.";

	/// Set synchronized from where the owner peer-status is known.
	synchronized = false;
	thrustingRequested = false;
	resetRequested = false;
	boostRequested = false;
};

RacingShipGlobal::~RacingShipGlobal()
{
	if (ai)
		delete ai;
	ai = NULL;
}

/// Function when entering this state.
void RacingShipGlobal::OnEnter(){

	/// Attach shield if not already done!
	if (entity->graphics == NULL){
	    /// Shield!
		GraphicsProperty * graphics = new GraphicsProperty();
	//	graphics->effects = new List<GraphicEffect*>();
	//	AlphaModelEffect * gfx = new AlphaModelEffect("Shield", "sphere", entity);
	//	graphics->effects->Add(gfx);

		/// Attach a light o-o
		graphics->dynamicLights = new List<Light*>();
		Light * headlights = new Light();
		headlights->type = LightType::SPOTLIGHT;
		headlights->relativeSpotDirection = Vector3f(0,0,-1.0f);
		float lightIntensity = 7.0f;
		headlights->attenuation = Vector3f(1.0f / lightIntensity, 0.001f, 0.00000001f);
		headlights->diffuse = Vector3f(0.8f,0.9f,1.f);
		headlights->specular = headlights->diffuse * 0.5f;
		headlights->spotCutoff = 65.0f;
		headlights->spotExponent = 12;
		headlights->owner = entity;
		graphics->dynamicLights->Add(headlights);

        /// Engine exhaust!
        graphics->particleSystems = new List<ParticleSystem*>();
        Exhaust * ps = new Exhaust(entity);
		graphics->particleSystems->Add(ps);
        exhaust = ps;
		exhaust->emissionsPerSecond = 10000 / (PlayerMan.NumPlayers()? PlayerMan.NumPlayers() : 1);
		exhaust->relativePosition = shipType->thrusterPosition;
		OnAccelerationUpdated();
		/// Attach the new graphicsProperty to the entity
		entity->graphics = graphics;

		// Don't scale down the shield, yo..
	//	Graphics.QueueMessage(new GMSetGraphicEffect(RELATIVE_SCALE, "Shield", Vector3f(0.7f,0.7f,0.7f), entity));

	}
}

/// Main processing function
void RacingShipGlobal::Process(float timePassed){

	/// Let host manage boost?
	if (!synchronized){
		boostRemaining += boostRegen * timePassed;
		if (boostRemaining > boostMax)
			boostRemaining = boostMax;

		if (boosting){
			boostRemaining -= timePassed;
			if (boostRemaining < 0){
				boostRemaining = 0.0f;
				boosting = false;
			//	std::cout<<"\nBoost empty!";
				// Stop boosting
				OnAccelerationUpdated();
			}
		}
	}
	/// Process AI if applicable.
	if (ai && ai->enabled)
		ai->Process(timePassed);
}
/// Function when leaving this state
void RacingShipGlobal::OnExit(){
}

void RacingShipGlobal::ProcessMessage(Message * message){
//	std::cout<<"\nRacingShipGlobal::ProcessMessage: ";
	switch(message->type){
		case MessageType::COLLISSION_CALLBACK: {
			CollissionCallback * c = (CollissionCallback*)message;

			/*
		//	std::cout<<"\nCollissionCallback received for entity "<<c->one->name<<" and "<<c->two->name;
			if (c->impactVelocity > 3.0f){
				/// Give a minor effect even from those entities that normally don't collide..!
				if (c->one->physics->noCollissionResolutions || c->two->physics->noCollissionResolutions)
					Graphics.QueueMessage(new GMSetGraphicEffect(ALPHA, "Shield", c->impactVelocity / 10000.0f, entity));
				else
					Graphics.QueueMessage(new GMSetGraphicEffect(ALPHA, "Shield", c->impactVelocity / 1000.0f, entity));
			}
			*/
			break;
		}
	}
}

/// Packs in all relevant data into a string. Packs different data if currently host or not, since host decides things.
String RacingShipGlobal::GetStateAsString(bool isHost){
	List<String> states;
	/// Relative, e.g. via joystick or axis-sticks.
	if (this->useRelativeValues){
		states += "RelativeThrust:"+String::ToString(this->relativeThrust);
		states += "RelativeTurn:"+String::ToString(this->relativeTurn);
	}
	/// Static, e.g. via keyboard.
	else {
		if (thrustingRequested)
			states += "ThrustingRequested";
		if (this->thrusting && !synchronized)
			states += "Thrust";
		if (this->reversing)
			states += "Reverse";
		else
			states += "Stop";
		if (this->left && !right)
			states += "Left";
		else if (right && !left)
			states += "Right";
		else
			states += "StopTurn";
	}
	/// For requesting host to enable/disable our boost.
	if (boostRequested)
		states += "BoostRequested";
	else
		states += "StopBoosting";
	/// Only send boost stuff if we are host.
	if (isHost){
		if (boosting)
			states += "Boost";
		else
			states += "StopBoost";
		states += "BoostRemaining:"+String::ToString(boostRemaining);
	}
	if (resetRequested){
		states += "Reset";
		resetRequested = false;
	}
	String r = MergeLines(states, ";");
	return r;
}

/// Loads from string as created via GetStateAsString
bool RacingShipGlobal::LoadStateFromString(String stateString){
	List<String> states = stateString.Tokenize(";");
	/// If local, don't do anything except accepting boost.
	if (this->entity->player->isLocal){
		/// Stop boosting by default, enable it only when Boost is packed into the packet.
		StopAccelerating(true);
		for (int i = 0; i < states.Size(); ++i){
			String s = states[i];
			if (s == "Thrust"){
				Accelerate(true);
			}
			else if (s == "Boost"){
				Boost(true);
			}
			else if (s == "StopBoost"){
				StopBoosting(true);
			}
			/// Fetch boost amount
			else if (s.Contains("BoostRemaining:")){
				boostRemaining = s.Tokenize(":")[1].ParseFloat();
			}

		}
		return true;
	}

	/// Reset state first, might as well.
	StopAccelerating(true);
	StopReversing();
	StopTurning();
//	StopBoosting();
	for (int i = 0; i < states.Size(); ++i){
		String s = states[i];
		if (s.Contains("RelativeThrust")){
			float value = s.Tokenize(":")[1].ParseFloat();
			Thrust(value);
		}
		else if (s.Contains("RelativeTurn")){
			float value = s.Tokenize(":")[1].ParseFloat();
			Turn(value);
		}
		if (s == "Thrust"){
			Accelerate(true);
		}
		if (s == "ThrustingRequested")
			Accelerate(true);
		else if (s == "Reverse")
			Reverse();
		if (s == "Left")
			TurnLeft();
		else if (s == "Right")
			TurnRight();
		if (s == "BoostRequested")
		{
			Boost(true);
		}
		else if (s == "StopBoosting"){
			StopBoosting(true);
		}
		/// Force boost, if we get a message with Boost it must be from host and should be good.
		if (s == "Boost"){
			Boost(true);
		}
		else if (s == "StopBoost")
			StopBoosting();
		/// Fetch boost amount
		if (s.Contains("BoostRemaining:")){
			boostRemaining = s.Tokenize(":")[1].ParseFloat();
		}
		if (s == "Reset")
			ResetPosition();
	}
	return true;
}

/// Wosh io-o
void RacingShipGlobal::RefillBoost(float amount){
	boostRemaining += amount;
	if (boostRemaining > boostMax)
		boostRemaining = boostMax;
}



// Acceleherating. If false will only generate a request if sychronized. If true will update graphics etc.
void RacingShipGlobal::Accelerate(bool force /*= false*/){
	if (this->synchronized && !force)
	{
		thrustingRequested = true;
		return;
	}
	if (thrusting)
		return;
	reversing = false;
//	std::cout<<"\nThrust!";
	relativeThrust = 1.0f;
	thrusting = true;
	useRelativeValues = false;
	OnAccelerationUpdated();
}

void RacingShipGlobal::StopAccelerating(bool force /* = false*/){
	if (this->synchronized && !force)
	{
		thrustingRequested = false;
		return;
	}
	if (!thrusting)
		return;
	relativeThrust = 0.0f;
//	std::cout<<"\nStopThrust!";
	thrusting = false;
	useRelativeValues = false;
	OnAccelerationUpdated();
}
void RacingShipGlobal::Reverse(){
	if (reversing)
		return;
	/// If thrusting, stop it.
	thrusting = false;
	relativeThrust = -1.0f;
//	std::cout<<"\nReverse!";
	reversing = true;
	useRelativeValues = false;
	OnAccelerationUpdated();
}
void RacingShipGlobal::StopReversing(){
	if (!reversing)
		return;
	relativeThrust = 0.0f;
//	std::cout<<"\nStopReverse!";
	reversing = false;
	useRelativeValues = false;
	OnAccelerationUpdated();
}
void RacingShipGlobal::TurnLeft(){
	if (left)
		return;
	right = false;
	relativeTurn = -1.0f;
	useRelativeValues = false;
	left = true;
	OnTurningUpdated();
}
void RacingShipGlobal::StopTurnLeft(){
	if (!left)
		return;
	relativeTurn = 0.0f;
	useRelativeValues = false;
	left = false;
	OnTurningUpdated();
}
void RacingShipGlobal::TurnRight(){
	if (right)
		return;
	left = false;
	relativeTurn = 1.0f;
	useRelativeValues = false;
	right = true;
	OnTurningUpdated();
}
void RacingShipGlobal::StopTurnRight(){
	if (!right)
		return;
	relativeTurn = 0.0f;
	right = false;
	useRelativeValues = false;
	OnTurningUpdated();
}

void RacingShipGlobal::StopTurning(){
	if (!right && !left)
		return;
	relativeTurn = 0.0f;
	right = left = false;
	useRelativeValues = false;
	OnTurningUpdated();
}

/// Relative ones, behave similarly to the above ones but more precisely. values from -1.0 to 1.0 are accepted.
void RacingShipGlobal::Thrust(float relative){
	assert(relative >= -1.0f && relative <= 1.0f);
	if (relative > 0)
		thrusting = true;
	else if (relative < 0)
		reversing = true;
	relativeThrust = relative;
	useRelativeValues = true;
	OnAccelerationUpdated();
}
void RacingShipGlobal::Turn(float relative){
	assert(relative >= -1.0f && relative <= 1.0f);
	if (relative < 0){
		left = true;
		right = false;
	}
	else if (relative > 0){
		left = false;
		right = true;
	}
	relativeTurn = relative;
	useRelativeValues = true;
	OnTurningUpdated();
}

/// If force, it will process anyway. If not, it will only send a request if it's synchronized.
void RacingShipGlobal::Boost(bool force){
	if (synchronized && !force)
	{
		boostRequested = true;
		return;
	}
	if (boosting)
		return;
	if (boostRemaining <= 1.0f)
		return;
//	std::cout<<"\nBoost!";
	boosting = true;
	OnAccelerationUpdated();
}

/// If force, it will process anyway. If not, it will only affect the boostRequested variable.
void RacingShipGlobal::StopBoosting(bool force){
	if (synchronized && !force){
		boostRequested = false;
		return;
	}
	if (!boosting)
		return;
//	std::cout<<"\nStopped boosting.";
	boosting = false;
	OnAccelerationUpdated();
}

void RacingShipGlobal::ResetPosition(){
	// Check if this is a synchronization type. If so queue a reset-request for the next update packet.
	if (this->synchronized){
		resetRequested = true;
		return;
	}

	time_t currentTime = Timer::GetCurrentTimeMs();
	if (lastResetTime + resetCooldown > currentTime)
		return;

	lastResetTime = currentTime;
	// Resetting position..!
	SRPlayer * player = (SRPlayer*)entity->player;
	player->checkpointsPassed;
	Entity * checkpoint = Racing::GetCheckpoint(player->checkpointsPassed-1);
	Vector3f position, rotation;
	if (checkpoint){
		position = checkpoint->position;
		rotation = checkpoint->rotation;
		rotation.y += PI;
	}
	/// If the map lacks checkpoints, use starting position!
	else {
		position = startingPosition;
		rotation = startingRotation;
	}
	Physics.QueueMessage(new PMSetEntity(VELOCITY, entity, entity->physics->velocity * 0.1f));
	Physics.QueueMessage(new PMSetEntity(SET_ROTATION, entity, rotation));
	Physics.QueueMessage(new PMSetEntity(POSITION, entity, position));
	if (ai)
		((AIRacer*)ai)->Reset();
}

void RacingShipGlobal::ToggleAI(){
	ai->enabled = !ai->enabled;
	((AIRacer*)ai)->Reset();
}

void RacingShipGlobal::OnAccelerationUpdated(){
	Vector3f relativeAcc;
	// Relative thrust
	if (useRelativeValues){
		if (relativeThrust > 0){
			relativeAcc.z = - relativeThrust * shipType->thrust;
		}
		else {
			relativeAcc.z = relativeThrust * shipType->reverse;
		}
		exhaust->emissionRatio = AbsoluteValue(relativeThrust) * 0.9f + 0.1f;
		if (boosting)
			exhaust->emissionRatio += 0.4f;
	}
	// Constant thrust
	else {
		if (thrusting){
			relativeAcc.z += - shipType->thrust;
			exhaust->emissionRatio = 1.0f;
			exhaust->emissionVelocity = 0.2f;
		   // exhaust->ResumeEmission();
		}
		else {
			exhaust->emissionRatio = 0.1f;
			exhaust->emissionVelocity = 0.1f;
		  //  exhaust->PauseEmission();
		}
		if (boosting){
			exhaust->emissionRatio += 0.4f;
			exhaust->emissionVelocity += 0.1f;
		}
		if (reversing)
			relativeAcc.z += shipType->reverse;
	}
	// Apply boost
	if (boosting && this->boostRemaining > 0){
		relativeAcc *= shipType->boostMultiplier;
		exhaust->SetColor(Vector3f(0.4f, 0.2f, 0.4f));
	}
	else {
		exhaust->SetColor(Vector3f(0.1f, 0.2f, 0.4f));
		boosting = false;
	}
//	std::cout<<"\nRacingShipGlobal::OnAccelerationUpdated: Setting acceleration to: "<<relativeAcc;
	Physics.QueueMessage(new PMSetEntity(ACCELERATION, entity, relativeAcc));
}
void RacingShipGlobal::OnTurningUpdated(){
	Vector3f relativeAngularAcc;
	/// Relative turning
	if (useRelativeValues){
		relativeAngularAcc.y = - relativeTurn * shipType->angularThrust;
	}
	/// Constant turning.
	else {
		if (left)
			relativeAngularAcc.y += shipType->angularThrust;
		if (right)
			relativeAngularAcc.y -= shipType->angularThrust;
	}
	// Send message and apply graphical effects if any.
	Physics.QueueMessage(new PMSetEntity(ANGULAR_ACCELERATION, entity, relativeAngularAcc));
}


// Reloads stats from the ship-type, like thruster statistics.
void RacingShipGlobal::ReloadFromShip(){
	exhaust->relativePosition = shipType->thrusterPosition;
}

/// Set starting position, to be used in-case checkpoints fail.
void RacingShipGlobal::SetStartingPosition(Vector3f position, Vector3f andRotation)
{
	startingPosition = position;
	startingRotation = andRotation;
}
