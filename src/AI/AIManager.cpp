///// Emil Hedemalm
///// 2013-02-08
//
//#include "AIManager.h"
//#include "EntityProperty.h"
//#include "AIState.h"
//#include "../Globals.h"
//#include "../Entity/Entity.h"
//#include "Entity/Entities.h"
//#include <ctime>
//#include <cassert>
//
//
//#define AI_MUTEX_NAME	L"AIManagerMutex"
//
//AIManager * AIManager::aiManager = NULL;
//
//// static const int MAX_AI_ENTITIES = 2048;	
//
///// Allocate
//void AIManager::Allocate(){
//	assert(aiManager == NULL);
//	aiManager = new AIManager();
//}
//AIManager * AIManager::Instance(){
//	assert(aiManager);
//	return aiManager;
//}
//void AIManager::Deallocate(){
//	assert(aiManager);
//	delete(aiManager);
//	aiManager = NULL;
//}
//
//AIManager::AIManager(){
//	lastUpdate = 0;
//	paused = false;
//	aiEntities = 0;
//	for (int i = 0; i < MAX_AI_ENTITIES; ++i)
//		aiEntity[i] = NULL;
//	simulationSpeed = 1.0f;
//
//	// Create mutex for AIManager too..
//	aiMutex.Create(AI_MUTEX_NAME);
//	/*HANDLE mutex = CreateMutex(NULL, false, AI_MUTEX_NAME);
//	if (!mutex){
//		int error = GetLastError();
//		if (error != NO_ERROR)
//			assert(false && "\nError when creating mutex:" && error);
//	}*/
//}
//AIManager::~AIManager(){
//
//}
//	
///// Performs various tests in order to optimize performance during runtime later. 
//void AIManager::Initialize(){
//}
//
//
///// Notification from the PathManager/WaypointManager that one or more paths may be invalid.
//void AIManager::PathsInvalidated(){
//	for (int i = 0; i < aiEntities; ++i){
//		//aiEntity[i]->state->PathsInvalidated();
//	}
//}
//
///** Registers an Entity to take part in physics calculations. This requires that the Entity has the physics attribute attached.
//	Returns 0 upon success, 1 if it's lacking a physics attribute, 2 if the Entity array has been filled and 3 if the dynamic entity array has been filled.
//*/
//int AIManager::RegisterEntity(Entity* newEntity){
//	if (newEntity->state == NULL){
//		if (false/*autoAddPhysics*/){
//		}
//		else
//			return 1;
//	}
//	bool success = false;
//	for (int i = 0; i < MAX_AI_ENTITIES; ++i){
//		if (aiEntity[i] == NULL){
//			aiEntity[i] = newEntity;
//			++aiEntities;
//			success = true;
//			break;
//		}
//		// Check also that it isn't already registered...!
//		else if (aiEntity[i] == newEntity){
//			ReregisterEntity(newEntity);
//			return 0;
//		}
//	}
//	if (!success)
//		return 2;
//	return 0;
//}
///** Registers a selection of entities to take part in physics calculations. This requires that the entities have physics attributes attached.
//	Returns 0 upon success or a positive number equal to the amount of entities that it failed to register.
//*/
//int AIManager::RegisterEntities(Entities & selection){
//	int failedRegistrations = 0;
//	for (int i = 0; i < selection.Size(); ++i){
//		if (RegisterEntity(selection[i]) != 0)
//			++failedRegistrations;
//	}
//	return failedRegistrations;
//}
//
//
///// Unregisters an Entity from the physics calculations. Returns 0 if it found the Entity and successfully removed it, 1 if not.
//int AIManager::UnregisterEntity(Entity* entityToRemove){
//	int found = 0;
//	for (int i = 0; i < MAX_AI_ENTITIES; ++i){
//		if (aiEntity[i] == entityToRemove){
//			--aiEntities;
//			/// If at end of list, make null-pointer
//			if (i == aiEntities)	
//				aiEntity[i] = NULL;
//			/// If not, move in last entity to have no spaces in the list
//			else {
//				aiEntity[i] = aiEntity[aiEntities];		// Move back the last entity for a compact array.
//				aiEntity[aiEntities] = NULL;
//			}
//			++found;
//		}
//	}
//	if (found > 1)
//		std::cout<<"\nINFO: "<<found<<" copies of same entity unregistered from Physics.";
//	else if (found == 0)
//		return -1;
//	return 0;
//}
//
///** Unregisters a selection of entities from physics calculations.
//	Returns 0 upon success or a positive number equal to the amount of entities that it failed to unregister.
//*/
//int AIManager::UnregisterEntities(Entities & selection){
//	int failedUnregistrations = 0;
//	for (int i = 0; i < selection.Size(); ++i){
//		if (UnregisterEntity(selection[i]) != 0)
//			++failedUnregistrations;
//	}
//	return failedUnregistrations;
//}
//
///// Unregisters all entities from physics calculations, and clears the collission vfcOctree as well.
//int AIManager::UnregisterAllEntities(){
//	int failedUnregistrations = 0;
//	int entities = this->aiEntities;
//	for (int i = 0; i < entities; ++i){
//		if (UnregisterEntity(this->aiEntity[0]) != 0)
//			++failedUnregistrations;
//	}
//	return failedUnregistrations;
//}
//
///// Executes queued state changes and processes all entities by their respective StateMan.
//void AIManager::Process(){
//	aiMutex.Claim(-1);
//	/*HANDLE mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, AI_MUTEX_NAME);
//	if (!mutex){
//		int error = GetLastError();
//		assert(error && "\nUnable to open AIManager mutex ");
//		return;
//	}
//	DWORD result = WaitForSingleObject(mutex, 10);
//
//	assert(result == WAIT_OBJECT_0 && "Unable to get Mutex in AIManager::Process()");
//	/// If we can't get mutex, just return!
//	if (result != WAIT_OBJECT_0)
//		return;*/
//	// Mutex gotten ^w^
//	if (paused)
//		return;
//	static int lastUpdate = 0;
//	int newTime = clock();
//	int timeDiff = newTime - lastUpdate;
//
//	/// Throw away time if we've got more than 1 second, since this assumes we're debugging
//	if (timeDiff > CLOCKS_PER_SEC / 10){
//		if (timeDiff > CLOCKS_PER_SEC)
//			std::cout<<"\nAIManager::Throwing away "<<timeDiff / CLOCKS_PER_SEC<<" debugging seconds";
//		timeDiff %= CLOCKS_PER_SEC / 10;
//	}
//
//	lastUpdate = newTime;
//	Entity* entity = NULL;
//	timeDiff *= simulationSpeed;
//
//
//	/// Just return if simulation speed decreases beyond 1%!
//	if (simulationSpeed <= 0.01f){
//		return;
//	}
//	
//	for (int i = 0; i < aiEntities; ++i){
//		entity = aiEntity[i];
//		///// Process queued state swaps
//		//if (entity->state->queuedState != NULL){
//		//	// Re-point stuff
//		//	if (entity->state->previousState)
//		//		delete entity->state->previousState;
//		//	entity->state->previousState = entity->state->currentState;
//		//	entity->state->currentState = entity->state->queuedState;
//		//	entity->state->queuedState = NULL;
//		//	// Process onExit/onEnter
//		//	entity->state->previousState->interaction.onExit(entity);
//		//	entity->state->currentState->OnEnter(entity);
//		//}
//		///// Process global state if any
//		if (entity->state->globalState){
//			entity->state->globalState->Process(entity, timeDiff);
//		}
//		/// Process current state
//		if (entity->state->currentState){
//		//	std::cout<<"\nAI: Processing "<<entity->name<<"..."<<" "<<entity->state->currentState->name;
//			entity->state->currentState->Process(entity, timeDiff);
//		//	std::cout<<"!";
//		}
//	}
//	/// Release AI mutex
//	aiMutex.Release();
//	/*int releaseResult = ReleaseMutex(mutex);
//	assert(releaseResult && "Unable to release AIManager::Process Mutex");
//	CloseHandle(mutex);*/
//}
//
///// Sets simulation playback multiplier.
//void AIManager::SetSpeed(float speed){
//	simulationSpeed = speed;
//}
//
///// Resumes physics calculations, moving entities in the world using gravitation, given velocities, etc.
//void AIManager::Resume(){
//	lastUpdate = clock();	// Fetch new time or time will fly by when we resume now :P
//	paused = false;
//}
///// Pauses physics calculations, sleeping the thread (if any) until resume is called once again.
//void AIManager::Pause(){
//	paused = true;
//}
///*
//
///// Processes queued messages.
//void AIManager::ProcessMessages(){
//	// Process queued messages
//	while (!messageQueue.isOff()){
//		AIMessage * msg = messageQueue.Pop();
//		msg->Process();
//		delete msg;
//	}
//}*/