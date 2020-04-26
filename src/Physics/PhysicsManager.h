/// Emil Hedemalm
/// 2013-03-07

#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

//#include "../PhysicsLib.h"
#include "PhysicsLib.h"
#include "PhysicsLib/Intersection.h"
#include "PhysicsOctree.h"
#include "Messages/PhysicsMessage.h"
#include <Util.h>
#include <Mutex/Mutex.h>

class CollisionDetector;
class Integrator;
class CollisionResolver;
class PhysicsMessage;
class AABBSweeper;
class Mesh;
struct Contact;
class Spring;
class PhysicsMesh;
class Ray;

#define PhysicsMan	(*PhysicsManager::Instance())
#define Physics		PhysicsMan

/// Checktypes
#define OCTREE      1
#define AABB_SWEEP  2


/// Integrators
namespace IntegratorType {
enum Integrators {
	/// Physics as calculated using simplified physics of only velocities and no acceleration forces whatsoever.
	SIMPLIFIED_PHYSICS, SIMPLE_PHYSICS = SIMPLIFIED_PHYSICS,
    /// Physics as calculated with strict Rigid body physics
	LAB_PHYSICS,
	/// Physics as I designed it earlier. Probably only viable for the Space Race project or other similar games with semi-wonky physics, lol.
	APPROXIMATE,
	SPACE_RACE_CUSTOM_INTEGRATOR = APPROXIMATE,
};};

/// Collision Resolvers
namespace CollisionResolverType{
enum collisionResolvers {
	CUSTOM_SPACE_RACE_PUSHBACK,
	LAB_PHYSICS_IMPULSES,
};};

#define MessageQueueP (PhysicsMan.mesManMessages)

/// Current time for this physics-frame. Updated in physics thread.
/// Set current time in physics for this frame. This time is not the same as real time.
extern int64 physicsNowMs;

class PhysicsManager
{
	friend class GraphicsManager;
	friend class PhysicsOctree;
	friend class PhysicsMessage;
	friend class PMSet; friend class PMSeti;
	friend class PMRegisterEntity; friend class PMRecalculatePhysicsMesh;
	friend class PMRegisterEntities;
	friend class PMUnregisterEntity;
	friend class PMUnregisterEntities;
	friend class PMSetPhysicsType;
	friend class PMSetPhysicsShape;
	friend class PMSetEntity;
	friend class PMSetSpeed;
	friend class CollisionShapeOctree;
	friend class PMRaycast;
private:
	PhysicsManager();
	static PhysicsManager * physicsManager;
public:

	int RegisteredEntities();

	/// Called once per frame from controlling thread. Handles message-processing, integration, simulation.
	void Process();

	/// Messages to queue back to the Message-manager upon processing of our own messages, including collision-detection, etc.
	List<Message*> mesManMessages;

	/// Chosen integrator.
	Integrator * physicsIntegrator;
	CollisionResolver * collisionResolver;
	CollisionDetector * collisionDetector;

	// Integrators
	void LabPhysicsIntegrate(EntitySharedPtr entity, float timeSinceLastUpdate);
	void ApproximateIntegrate(EntitySharedPtr entity, float timeSinceLastUpdate);

	void DetectCollisions();

	void RecalculateAABBs();
	void RecalculateOBBs();

	// Grab AABB of all relevant entities? Check the AABB-sweeper or other relevant handler?
	AABB GetAllEntitiesAABB();

    /// See above.
    /// Defines if AABBs or sphere-octrees should be used to broad-phase collission detection.
    int checkType;
    /// Defines how collissions will be resolved.
    int collisionResolverType;
	/// How stuff is updated.
	int integratorType;

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
	// Enters a message into the message queue
	void QueueMessages(List<PhysicsMessage*> msgs);

	/// Attaches a physics property to target entity if it didn't already have one.
	void AttachPhysicsTo(EntitySharedPtr entity);

	/// Resumes physics calculations, moving entities in the world using gravitation, given velocities, etc.
	void Resume();
	/// Pauses physics calculations, sleeping the thread (if any) until resume is called once again.
	void Pause();
	/// Returns the pause StateMan.
	bool IsPaused() const { return paused; };

	/// Returns a list of all registered entities.
	Entities GetEntities();
	List< std::shared_ptr<Entity> > GetDynamicEntities();
	const Vector3f Gravity() const { return gravitation; };

	/// Numeric statistics
	inline float GetPhysicsMeshCollisionChecks() const { return physicsMeshCollisionChecks; };

    /// Loads physics mesh if not already loaded.
    void EnsurePhysicsMesh(EntitySharedPtr targetEntity);
	
	Collision lastCollision;

	bool PauseOnCollision() const { return pauseOnCollision; };

	/// List of all active (or in-active) contacts in-game. Stored here so that deletion works bettar?
	List<Contact*> contacts;
	/// List of all active (or in-active) springs in-game. Stored here so that deletion will be handled correctly by the manager.
	List<Spring*> springs;
	/// In kg per m^3
	float defaultDensity;

	/// Numeric statistics
	float physicsMeshCollisionChecks;

	Vector3f GetGravitation() { return gravitation; };

	Entities forceBasedEntities;

	/// Casts a ray, returns result of sorted intersections by distance. First index is the closest intersection. Only to be called from physics thread.
	List<Intersection> Raycast(Ray & ray);

private:

	/// Functions for handling the various aspects of the physical simulation procedures.
	void Integrate(float timeInSecondsSinceLastUpdate);
	void ApplyConstraints();
	/// Applies pathfinding for all relevant entities
	void ApplyPathfinding();

	bool pauseOnCollision;

	/// Loads PhysicsMesh from mesh counterpart
	PhysicsMesh * LoadPhysicsMesh(Mesh * byMeshSource);
	/// Getter funcsschtlions
	PhysicsMesh * GetPhysicsMesh(const String bySourceFile);
	PhysicsMesh * GetPhysicsMesh(const Mesh * byMeshSource);
	/// Called server-side.
	void RecalculatePhysicsMesh(Mesh * mesh);
	/// Checks if the entity requires a physics mesh and loads it if so.
	void EnsurePhysicsMeshIfNeeded(EntitySharedPtr targetEntity);

	/// Meshes used for collission calculations
	List<PhysicsMesh*> physicsMeshes;

	/// List of triangles active in collissions. Cleared each frame.
	List<Triangle> activeTriangles;

	/// Mutex to be used for accessing the message queue.
	Mutex physicsMessageQueueMutex;

	/// Simulation speed multiplier
	float simulationSpeed;
	/// Flag for skipping collission-calculations.
	bool ignoreCollisions;

	/** Recalculates physical properties for all registered entities. */
	void RecalculatePhysicsProperties();

	/// Processes queued messages.
	void ProcessMessages();
	/// Processes physics for all registered objects
	void ProcessPhysics();

	/// Sets physics type of target entity.
	void SetPhysicsType(EntitySharedPtr entity, int type);
	/// Sets physics type of target entities.
	void SetPhysicsType(List< std::shared_ptr<Entity> > &targetEntities, int type);
	/// Sets physics shape (Plane, Sphere, Mesh, etc.)
	void SetPhysicsShape(List< std::shared_ptr<Entity> > targetEntities, int type);

	/** Registers an Entity to take part in physics calculations. This requires that the Entity has the physics attribute attached.
		Returns 0 upon success, 1 if it's lacking a physics attribute, 2 if the Entity array has been filled and 3 if the dynamic entity array has been filled.
		4 if it's already registered.
	*/
	int RegisterEntity(EntitySharedPtr Entity);
	/** Registers a selection of entities to take part in physics calculations. This requires that the entities have physics attributes attached.
		Returns 0 upon success or a positive number equal to the amount of entities that it failed to register.
	*/
	int RegisterEntities(List< std::shared_ptr<Entity> >& targetEntities);
	/// Unregisters and re-registers selected entity.
	void ReregisterEntity(EntitySharedPtr entity) { UnregisterEntity(entity); RegisterEntity(entity); };
	/// Unregisters an Entity from the physics calculations. Returns 0 if it found the Entity and successfully removed it, 1 if not.
	int UnregisterEntity(EntitySharedPtr Entity);
	/** Unregisters a selection of entities from physics calculations.
		Returns 0 upon success or a positive number equal to the amount of entities that it failed to unregister.
	*/
	int UnregisterEntities(List< std::shared_ptr<Entity> > &targetEntities);
	/** Unregisters all entities from physics calculations, and clears the collission vfcOctree as well.
		Returns 0 upon success or a positive number equal to the amount of entities that it failed to
		unregister. */
	int UnregisterAllEntities();

	// Queue for messages to be processed between renders
	List<PhysicsMessage*> messageQueue;

	/// Physics collission octree for minimizing amount of collission detection checks.
	PhysicsOctree * entityCollisionOctree;
	AABBSweeper * aabbSweeper;

	/// If calculations should pause.
	bool paused;
	/// Time in milliseconds that last physics update was performed.
	time_t lastUpdate;

	/// Damping applied each frame. 1.0 = retain all speed. Suggested values around 0.5 for starters. Ratio velocities are multiplied per second.
	float linearDamping;
	float angularDamping;


	/// Number of registered entities
	Entities physicalEntities,
		dynamicEntities,
		kinematicEntities,
		fullyDynamicEntities, // Most non-static ones?
		semiDynamicEntities;  // Those entities which are KINEMATIC or DYNAMIC but have the fullyDynamic flag set to false.
/*	/// Old manual list implementation~~
	/// Array with pointers to all registered objects.
	EntitySharedPtr physicalEntity[MAX_REGISTERED_ENTITIES];
	/// Amount of currently registered objects.
	int physicalEntities;

	/// Array with pointers to all registered dynamic objects.
	EntitySharedPtr dynamicEntity[MAX_DYNAMIC_ENTITIES];
	/// Amount of currently registered dynamic entities.
	int dynamicEntities;
*/

	/// Gravitation vector, may be altered with functions later if wished.
	Vector3f gravitation;
	/// Density of air.
	float airDensity;

};


#endif
