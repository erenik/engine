///// Emil Hedemalm
///// 2013-02-08
//
//#ifndef AI_MANAGER_H
//#define AI_MANAGER_H
//
//#define MAX_AI_ENTITIES	500
//
//struct AIMessage;
//class Entities;
//class Entity;
//
//#define AI			(*AIManager::Instance())
//
//#include <Mutex/Mutex.h>
//
//// Manager that handles registration and processing of all actively AId entities.
//class AIManager {
//private:
//	// Constructor and destructor
//	AIManager();
//	static AIManager * aiManager;
//public:
//	// Returns the instance of the AIManager
//	static void Allocate();
//	static AIManager * Instance();
//	static void Deallocate(); 
//	// Destructor might need to be public
//	~AIManager();
//
//	/// Allocates all necessary global AI controls.
//	void Initialize();
//
//	/// Notification from the PathManager/WaypointManager that one or more paths may be invalid.
//	void PathsInvalidated();
//
//	/** Registers an Entity to take part in AI calculations. This requires that the Entity has the AI attribute attached.
//		Returns 0 upon success, 1 if it's lacking an AI attribute, 2 if the Entity array has been filled and 3 if the dynamic entity array has been filled.
//	*/
//	int RegisterEntity(Entity* Entity);
//	/** Registers a selection of entities to take part in AI calculations. This requires that the entities have AI attributes attached.
//		Returns 0 upon success or a positive number equal to the amount of entities that it failed to register.
//	*/
//	int RegisterEntities(Entities & selection);
//	/// Unregisters and re-registers selected entity.
//	void ReregisterEntity(Entity* entity) { UnregisterEntity(entity); RegisterEntity(entity); };
//	/// Unregisters an Entity from the AI calculations. Returns 0 if it found the Entity and successfully removed it, 1 if not.
//	int UnregisterEntity(Entity* Entity);
//	/** Unregisters a selection of entities from AI calculations.
//		Returns 0 upon success or a positive number equal to the amount of entities that it failed to unregister.
//	*/
//	int UnregisterEntities(Entities & selection);
//	/** Unregisters all entities from AI calculations.
//		Returns 0 upon success or a positive number equal to the amount of entities that it failed to 
//		unregister. */
//	int UnregisterAllEntities();
//
//	/// Executes queued state changes and processes all entities by their respective StateMan.
//	void Process();
//
//	/// Sets simulation playback multiplier.
//	void SetSpeed(float speed);
//	/// Halts all AI processing
//	void Pause();
//	/// Resumes AI processing
//	void Resume();
//
//private:
//	Mutex aiMutex;
//	/// Defines playback simulation speed.
//	float simulationSpeed;
//
//	// Queue for messages to be processed between renders
////	Queue<AIMessage*> messageQueue;
//
//	/// If calculations should pause.
//	bool paused;
//	/// Time in milliseconds that last physics update was performed.
//	long lastUpdate;
//	
//	/// Array with pointers to all registered dynamic objects.
//	Entity* aiEntity[MAX_AI_ENTITIES];
//	/// Amount of currently registered dynamic entities.
//	int aiEntities;
//};
//
//#endif
