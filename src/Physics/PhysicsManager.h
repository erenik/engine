/// Emil Hedemalm
/// 2013-03-07

#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#include "../PhysicsLib.h"
#include "PhysicsOctree.h"
#include "Messages/PhysicsMessage.h"
#include <Util.h>
#include <Mutex/Mutex.h>

class PhysicsMessage;
class AABBSweeper;
class Mesh;
struct Contact;
class Spring;

#define Physics		(*PhysicsManager::Instance())


/// Checktypes
#define OCTREE      1
#define AABB_SWEEP  2

/// Collission Resolvers
#define CUSTOM_SPACE_RACE_PUSHBACK      1
#define LAB_PHYSICS_IMPULSES            2

/// Integrators
namespace Integrator {
enum Integrators {
	/// Physics as calculated using simplified physics of only velocities and no acceleration forces whatsoever.
	SIMPLIFIED_PHYSICS, SIMPLE_PHYSICS = SIMPLIFIED_PHYSICS,
    /// Physics as calculated with strict Rigid body physics
	LAB_PHYSICS,
	/// Physics as I designed it earlier. Probably only viable for the Space Race project or other similar games with semi-wonky physics, lol.
	SPACE_RACE_CUSTOM_INTEGRATOR,
};};

class PhysicsManager{
	friend class GraphicsManager;
	friend class PhysicsOctree;
	friend class PhysicsMessage;
	friend class PMSet;
	friend class PMSetGravity;
	friend class PMRegisterEntity;
	friend class PMRegisterEntities;
	friend class PMUnregisterEntity;
	friend class PMUnregisterEntities;
	friend class PMSetPhysicsType;
	friend class PMSetPhysicsShape;
	friend class PMSetEntity;
	friend class PMSetSpeed;
	friend class CollisionShapeOctree;
private:
	PhysicsManager();
	static PhysicsManager * physicsManager;
public:
    /// See above.
    /// Defines if AABBs or sphere-octrees should be used to broad-phase collission detection.
    int checkType;
    /// Defines how collissions will be resolved.
    int collissionResolver;
	/// How stuff is updated.
	int integrator;

	static void Allocate();
	static PhysicsManager * Instance();
	static void Deallocate();
	~PhysicsManager();

	/// Allocates the physics vfcOctree and performs various tests in order to optimize performance during runtime later.
	void Initialize();
	/// Initializes the vfcOctree to the specified size (cube).
	void InitOctree(float size);
	/// Initializes the vfcOctree to the specified bounds.
	void InitOctree(float leftBound, float rightBound, float topBound, float bottomBound, float nearBound, float farBound);

#define MAX_PHYSICAL_ENTITIES Physics::MAX_REGISTERED_ENTITIES

	static const int MAX_REGISTERED_ENTITIES = 2048;
	static const int MAX_DYNAMIC_ENTITIES = 256;

	// Enters a message into the message queue
	void QueueMessage(PhysicsMessage * msg);

	/// Attaches a physics property to target entity if it didn't already have one.
	void AttachPhysicsTo(Entity * entity);

	/// Resumes physics calculations, moving entities in the world using gravitation, given velocities, etc.
	void Resume();
	/// Pauses physics calculations, sleeping the thread (if any) until resume is called once again.
	void Pause();
	/// Returns the pause StateMan.
	bool IsPaused() const { return paused; };

	/// Returns a list of all registered entities.
	Selection GetEntities();
	List<Entity*> GetDynamicEntities();
	const Vector3f Gravity() const { return gravitation; };

	/// Debugging time statistics
	inline float GetMessageProcessingFrameTime() const {return messageProcessingTime; };
	inline float GetRecalculatingPropertiesFrameTime() const { return recalculatingPropertiesDuration; };
	inline float GetMovementFrameTime() const { return movingDuration; };
	inline float GetCollissionProcessingFrameTime() const { return collissionProcessingFrameTime; };
	/// Numeric statistics
	inline float GetPhysicsMeshCollissionChecks() const { return physicsMeshCollissionChecks; };

    /// Loads physics mesh if not already loaded.
    void EnsurePhysicsMesh(Entity * targetEntity);
	
	Collission lastCollission;

	bool PauseOnCollission() const { return pauseOnCollission; };

	/// List of all active (or in-active) contacts in-game. Stored here so that deletion works bettar?
	List<Contact*> contacts;
	/// List of all active (or in-active) springs in-game. Stored here so that deletion will be handled correctly by the manager.
	List<Spring*> springs;
	/// In kg per m^3
	float defaultDensity;
private:
	/// Functions for handling the various aspects of the physical simulation procedures.
	void Integrate(float timeSinceLastUpdate);
	void ApplyConstraints();
	/// Applies pathfinding for all relevant entities
	void ApplyPathfinding();

	/// Debugging time statistics
	float messageProcessingTime,
		recalculatingPropertiesDuration,
		movingDuration,
		collissionProcessingFrameTime;

	bool pauseOnCollission;

	/// Numeric statistics
	float physicsMeshCollissionChecks;

	/// Loads PhysicsMesh from mesh counterpart
	PhysicsMesh * LoadPhysicsMesh(const Mesh * byMeshSource);
	/// Getter funcsschtlions
	PhysicsMesh * GetPhysicsMesh(const String bySourceFile);
	PhysicsMesh * GetPhysicsMesh(const Mesh * byMeshSource);
	/// Checks if the entity requires a physics mesh and loads it if so.
	void EnsurePhysicsMeshIfNeeded(Entity * targetEntity);

	/// Meshes used for collission calculations
	List<PhysicsMesh*> physicsMeshes;

	/// List of triangles active in collissions. Cleared each frame.
	List<Triangle> activeTriangles;

	/// Mutex to be used for accessing the message queue.
	Mutex physicsMessageQueueMutex;

	/// Simulation speed multiplier
	float simulationSpeed;
	/// Flag for skipping collission-calculations.
	bool ignoreCollissions;

	/** Recalculates physical properties for all registered entities. */
	void RecalculatePhysicsProperties();

    //  Following moved to Collission.h!
	/// Updates the entity's collission state (colliding, in rest, on plane) using it's current velocities and the collission normal.
///	void UpdateCollissionState(Entity * entity, Vector3f collissionNormal = Vector3f());
	/** Tests if a collission should occur between the two objects.
		If so, it will save the collission data into the data parameter and return true.
		If no collission should occur, it will return false.
	*/
	bool TestCollission(Entity * one, Entity * two, List<Collission> & collissionList);
	/// Processes queued messages.
	void ProcessMessages();
	/// Processes physics for all registered objects
	void ProcessPhysics();

	/// Sets physics type of target entity.
	void SetPhysicsType(Entity * entity, int type);
	/// Sets physics type of target entities.
	void SetPhysicsType(List<Entity*> &targetEntities, int type);
	/// Sets physics shape (Plane, Sphere, Mesh, etc.)
	void SetPhysicsShape(List<Entity*> &targetEntities, int type);

	/** Registers an Entity to take part in physics calculations. This requires that the Entity has the physics attribute attached.
		Returns 0 upon success, 1 if it's lacking a physics attribute, 2 if the Entity array has been filled and 3 if the dynamic entity array has been filled.
		4 if it's already registered.
	*/
	int RegisterEntity(Entity * Entity);
	/** Registers a selection of entities to take part in physics calculations. This requires that the entities have physics attributes attached.
		Returns 0 upon success or a positive number equal to the amount of entities that it failed to register.
	*/
	int RegisterEntities(List<Entity*>& targetEntities);
	/// Unregisters and re-registers selected entity.
	void ReregisterEntity(Entity * entity) { UnregisterEntity(entity); RegisterEntity(entity); };
	/// Unregisters an Entity from the physics calculations. Returns 0 if it found the Entity and successfully removed it, 1 if not.
	int UnregisterEntity(Entity * Entity);
	/** Unregisters a selection of entities from physics calculations.
		Returns 0 upon success or a positive number equal to the amount of entities that it failed to unregister.
	*/
	int UnregisterEntities(List<Entity*> &targetEntities);
	/** Unregisters all entities from physics calculations, and clears the collission vfcOctree as well.
		Returns 0 upon success or a positive number equal to the amount of entities that it failed to
		unregister. */
	int UnregisterAllEntities();

	// Queue for messages to be processed between renders
	Queue<PhysicsMessage*> messageQueue;

	/// Physics collission octree for minimizing amount of collission detection checks.
	PhysicsOctree * entityCollissionOctree;
	AABBSweeper * aabbSweeper;

	/// If calculations should pause.
	bool paused;
	/// Time in milliseconds that last physics update was performed.
	time_t lastUpdate;


	/// Number of registered entities
	List<Entity*> physicalEntities;
	List<Entity*> dynamicEntities;
/*	/// Old manual list implementation~~
	/// Array with pointers to all registered objects.
	Entity * physicalEntity[MAX_REGISTERED_ENTITIES];
	/// Amount of currently registered objects.
	int physicalEntities;

	/// Array with pointers to all registered dynamic objects.
	Entity * dynamicEntity[MAX_DYNAMIC_ENTITIES];
	/// Amount of currently registered dynamic entities.
	int dynamicEntities;
*/

	/// Gravitation vector, may be altered with functions later if wished.
	Vector3f gravitation;
	/// Density of air.
	float airDensity;

};


#endif
