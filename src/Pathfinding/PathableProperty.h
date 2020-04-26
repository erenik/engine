/// Emil Hedemalm
/// 2016-07-29
/// Property for anything using pathfinding system. NPCs, player, walking enemies. Relies on messages (see PathMessage.h). Does not start any actual movement by itself.

#ifndef PATHABLE_PROP
#define PATHABLE_PROP

#include "Entity/EntityProperty.h"
#include "Path.h"

class PathableProperty : public EntityProperty
{
public:
	PathableProperty(EntitySharedPtr owner);
	virtual ~PathableProperty();
	static int ID(){return EntityPropertyID::PATHABLE_PROPERTY;};
	
	/// Mainly handles.
	virtual void ProcessMessage(Message * message);
	virtual void Process(int timeInMs);
	
	/// Requests a path to the target waypoint. This will then be returned via the PathManaging system as a message.
	virtual void RequestPath(Waypoint * wp);
	/// Requests a path to the target position. This will then be returned via the PathManaging system as a message.
	virtual void RequestPath(ConstVec3fr toPosition);
	
	/// Sets new path to walk, first going to the first waypoint in the path.
	virtual void StartWalking(Path alongPath);
	/// To resume walking along path.
	virtual void ResumeWalking();
	/// Sets isWalking to false. Stops checking distance to next waypoint. Sends by default also a PT_SET_ACCELERATION message with 0 as argument. Subclass to override.
	/// Sends a String message with 'OnFinalDestinationSet' as text since the entity is approaching its destination, but may still not be there just yet.
	virtual void StopWalking();
	
	/// Query if it has reached next waypoint. Just compares distance with the proximity threshold.
	virtual bool HasReachedNextWaypoint();
	/// Sets next waypoint as target. If next waypoint becomes 0, it will stop walking. If path is cyclic, will never stop walking until StopWalking is called.
	/// Returns true if there is another waypoint. false if we are reaching the last point.
	virtual bool SetNextWaypointAsTarget();
	
	/// This should be subclassed to actually add some physics, based on your given game. By default it will send a PT_SET_ACCELERATION message to the entity using walkingAcceleration.
	virtual void GoToWaypoint(Waypoint * wp);
	
	/// Threshold to which it will consider the entity to have reached target waypoint. Default value in defaultProximityThreshold.
	float proximityThreshold;
	static float defaultProximityThreshold; // default 1.0f
	/// How long time should go between updates? (minimum) Default 1000ms (1 second)
	int updateIntervalMs;
	
private:
	/// If walking along path.
	bool isWalking;
	/// Relevant path.
	Path path;
	Waypoint * nextWaypoint;
	/// Accumulator.
	int msSinceLastUpdate;
};

#endif
