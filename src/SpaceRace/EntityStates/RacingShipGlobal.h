// 2013-06-15

#include "EntityStates/EntityState.h"

class Exhaust;
class Ship;

class RacingShipGlobal : public EntityState {
public:
	/// Owner and ship-blueprint to base our interactions on!
	RacingShipGlobal(Entity * owner, Ship * ship);
	virtual ~RacingShipGlobal();
	/// Function when entering this state.
	void OnEnter();
	/// Main processing function
	void Process(float timePassed);
	/// Function when leaving this state
	void OnExit();
	/// Function for handling messages sent to the entity.
	void ProcessMessage(Message * message);

	/// Packs in all relevant data into a string. Packs different data if currently host or not, since host decides things.
	String GetStateAsString(bool isHost);
	/// Loads from string as created via GetStateAsString
	bool LoadStateFromString(String stateString);

	/// Wosh io-o
	void RefillBoost(float amount);

	// Acceleherating. If false will only generate a request if sychronized. If true will update graphics etc. 
	void Accelerate(bool force = false);
	/// IF false will only negate the request. If true will update graphics etc. correctly.
	void StopAccelerating(bool force = false);
	void Reverse();
	void StopReversing();

	/// Turning.
	void TurnLeft();
	void StopTurnLeft();
	void TurnRight();
	void StopTurnRight();
	void StopTurning();

	/// Relative ones, behave similarly to the above ones but more precisely. values from -1.0 to 1.0 are accepted.
	void Thrust(float relative);
	void Turn(float relative);

	
	/// If force, it will process anyway. If not, it will only send a request if it's synchronized.
	void Boost(bool force = false);
	/// If force, it will process anyway. If not, it will only affect the boostRequested variable.
	void StopBoosting(bool force = false);

	/// If on the side >:
	void ResetPosition();

	void ToggleAI();

	// Getterrrr
	float RemainingBoost() const { return boostRemaining; };

	// Reloads stats from the ship-type, like thruster statistics.
	void ReloadFromShip();

	/// If true, this means this state belongs to a client-side synchronized entity, meaning requests are sent to the server for processing before for example Resets are invoked.
	bool synchronized;
	
	/// Set starting position, to be used in-case checkpoints fail.
	void SetStartingPosition(Vector3f position, Vector3f andRotation);

private:
	/// Position set at the start of a race. Usually somewhere close to the goal.
	Vector3f startingPosition;
	Vector3f startingRotation;

	/// o-o;
	void OnAccelerationUpdated();
	void OnTurningUpdated();

	/// From -1.0f to 1.0f, left being -1 and right 1
	float relativeThrust, relativeTurn;
	/// Defaults to 0. Changes if the relative or constant functions are used!
	bool useRelativeValues;

	/// States
	bool thrusting, reversing, left, right, boosting;
	/// Flagging to request host to reset our position. (requires host confirmation)
	bool thrustingRequested, resetRequested, boostRequested;

	/// Reference ship ^^
	Ship * shipType;
	/// Active particle emitter..! :)
	Exhaust * exhaust;
	/// Current amount of "boost" fuel available for usage.
	float boostRemaining;
	/// Amount regenerated per second.
	float boostRegen;
	/// Maximum amount of boost that can be stored.
	float boostMax;

	/// To avoid woshi.
	time_t lastResetTime, resetCooldown;

};
