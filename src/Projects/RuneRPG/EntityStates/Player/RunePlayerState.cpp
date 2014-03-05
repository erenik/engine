// Emil Hedemalm
// 2013-06-15

#include "RunePlayerState.h"

#include "Message/Message.h"
#include "Physics/Messages/CollissionCallback.h"
#include "EntityStates/EntityStates.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/GraphicsProperty.h"
#include "Maps/MapManager.h"
#include "Maps/2D/TileMap2D.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Physics/PhysicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Messages/GraphicsMessages.h"
#include "Physics/PhysicsProperty.h"
#include "Pathfinding/PathfindingProperty.h"

const float RunePlayerState::DEFAULT_MOVEMENT_SPEED = 0.25f;

RunePlayerState::RunePlayerState(Entity * entity)
: EntityStateTile2D(entity)
{
	movementSpeed = DEFAULT_MOVEMENT_SPEED;
	timePassedSinceLastMovement = 0;
	left = right = up = down = false;
	isMoving = false;
	direction = queuedDirection = Direction::NONE;
	movementProgress = 0.0f;
	movementEnabled = true;
};

/// Function when entering this state.
void RunePlayerState::OnEnter(){
}

/// Main processing function
void RunePlayerState::Process(float timePassed){
	// 
	timePassedSinceLastMovement += timePassed;
	
	/// Only process movement if enabled!
	if (movementEnabled)
		HandleMovement(timePassed);

}
	/// Function when leaving this state
void RunePlayerState::OnExit(){
}

void RunePlayerState::ProcessMessage(Message * message){
	std::cout<<"\nRunePlayerState::ProcessMessage: ";
	switch(message->type){
		case MessageType::STRING: {
			message->msg;
			String & s = message->msg;
			s.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (s == "walkLeft")
				left = true;
			else if (s == "walkRight")
				right = true;
			else if (s == "walkUp")
				up = true;
			else if (s == "walkDown")
				down = true;
			else if (s == "stopWalkLeft")
				left = false;
			else if (s == "stopWalkRight")
				right = false;
			else if (s == "stopWalkUp")
				up = false;
			else if (s == "stopWalkDown")
				down = false;
			else if (s == "Interact"){
				/// Check if there's any stuff on the map at looking location and trigger their OnInteract events if so!
				TileMap2D * tmap = (TileMap2D*)MapMan.ActiveMap();
				if (!tmap){
					std::cout<<"\nERROR: RunePlayerState::ProcessMessage: No Active map to interact on!";
					return;
				}
				Vector2i intTile = GetInteractionTile();
				/// Send along our entity as reference.
				tmap->Interact(intTile, entity);
			}
			break;						  
		}					  
		case MessageType::COLLISSION_CALLBACK: {
					
			break;
		}
	}	
	UpdateQueuedMovement();
}

/// Wosh.
void RunePlayerState::DisableMovement(){
	movementEnabled = false;
}
void RunePlayerState::EnableMovement(){
	movementEnabled = true;
}

/// One step in the looking-direction.
Vector2i RunePlayerState::GetInteractionTile(){		
	// Check if tile is vacant and walkable, if so move to it.
	Vector3f pos = entity->positionVector;
	pos.Round();
	int tileX = pos.x;
	int tileY = pos.y;

	int dir = lastQueuedDirection;
	if (dir == Direction::NONE)
		dir = previousDirection;
	switch(dir){
		case Direction::LEFT:
			tileX--;
			break;
		case Direction::RIGHT:
			tileX++;
			break;
		case Direction::UP:
			tileY++;
			break;
		case Direction::DOWN:
			tileY--;
			break;
	}
	Vector2i tile(tileX, tileY);
	return tile;
}

/// Based on the requested direction..!
void RunePlayerState::UpdateQueuedMovement(){
	if (up)
		queuedDirection = Direction::UP;
	else if (left)
		queuedDirection = Direction::LEFT;
	else if (right)
		queuedDirection = Direction::RIGHT;
	else if (down)
		queuedDirection = Direction::DOWN;
	else
		queuedDirection = Direction::NONE;
	/// If we have moment?
	if (queuedDirection != Direction::NONE){
		lastQueuedDirection = queuedDirection;
		// Inform the Physics/Pathfinder of it!
		Graphics.QueueMessage(new GMSetEntity(entity, ANIMATION, "Walk"));
	}
	/// And not?
	else {
		PathfindingProperty * path = entity->pathfindingProperty;
		path->desiredVelocity = Vector3f();
		std::cout<<"\nStopping.";
	}
}

void RunePlayerState::HandleMovement(float & timePassed){
	float timeRemainingAfterReachingDestination = 0.0f;

	if (isMoving){
		/// Check if we're there yet.
		assert(movementSpeed > 0);
		movementProgress += timePassed / movementSpeed;
		if (movementProgress > 1.0f){
			timeRemainingAfterReachingDestination = (movementProgress - 1.0f) * movementSpeed;
			movementProgress = 1.0f;
			isMoving = false;
			previousDirection = direction;
			direction = Direction::NONE;
			TileMap2D * map = (TileMap2D*)MapMan.ActiveMap();
			Vector3f pos = entity->positionVector;
			pos.Round();
			assert(false);
		//	map->OnArrive(entity, pos.x, pos.y);
			/// Update 2D-position.
			entityTile2D->position.x = pos.x;
			entityTile2D->position.y = pos.y;
			std::cout<<"\nUpdating position to: "<<entityTile2D->position.x<<", "<<entityTile2D->position.y;
		}
		/// Process the movement
		Vector3f framePosition = newPosition * movementProgress + previousPosition * (1 - movementProgress);
	//	Graphics.PauseRendering();
		Physics.QueueMessage(new PMSetEntity(POSITION, entity, framePosition));
	//	entity->recalculateMatrix();
	//	Graphics.ResumeRendering();
		
	}
	
	/// If we reached next tile, see if we got a queued direction and occupy that next tile if possible and if so! :)
	if (direction == Direction::NONE && queuedDirection != Direction::NONE){
		// Check if tile is vacant and walkable, if so move to it.
		Vector3f pos = entity->positionVector;
//		std::cout<<"\nPosition: "<<pos;
		
		previousPosition = pos;
		pos.Round();
		tileX = pos.x;
		tileY = pos.y;

		int reqTileX = tileX;
		int reqTileY = tileY;

		if (timePassedSinceLastMovement < movementSpeed)
			return;
		timePassedSinceLastMovement = 0;

		// 
		Vector3f requestedVelocity;
		switch(queuedDirection){
		case Direction::LEFT:
			reqTileX--;
			requestedVelocity.x = -1.0f;
			break;
		case Direction::RIGHT:
			reqTileX++;
			requestedVelocity.x = 1.0f;
			break;
		case Direction::UP:
			reqTileY++;
			requestedVelocity.y = 1.0f;
			break;
		case Direction::DOWN:
			requestedVelocity.y = -1.0f;
			reqTileY--;
			break;
		default:
			requestedVelocity = Vector3f();
			break;
		}
		requestedVelocity *= 5.0f;
		assert(entity->physics);

		/// Send requested velocity to the animation thingy!
		PathfindingProperty * path = entity->pathfindingProperty;
		path->desiredVelocity = requestedVelocity;
		/// Consider setting this somewhere else..
		entity->physics->boundToNavMesh = true;
		std::cout<<"\nSetting desired velocity to: "<<requestedVelocity;
		/// Update 2D-position.
		entityTile2D->position.x = pos.x;
		entityTile2D->position.y = pos.y;

		std::cout<<"\nUpdating position to: "<<entityTile2D->position.x<<", "<<entityTile2D->position.y;

/*
		/// Check if tile is walkeable and place as there if so.
		TileMap2D * map = (TileMap2D*)MapMan.ActiveMap();
		if (map->IsTileVacant(reqTileX, reqTileY)){
			std::cout<<"\nChecking if requested tile is vacant: "<<reqTileX<<" "<<reqTileY;
			bool success = map->MoveEntity(entity, reqTileX, reqTileY);
			if (success){
				newPosition = Vector3f(reqTileX, reqTileY, 0);
				std::cout<<"\nRunePlayerState::HandleMovement Moved to: "<<newPosition;
				direction = queuedDirection;
				movementProgress = 0.0f;
				isMoving = true;
			}
			
		}
*/
	//	else {
		if (entity->physics->velocity.MaxPart() <= 0.1f)
			Graphics.QueueMessage(new GMSetEntity(entity, ANIMATION, "Idle"));
	//	}
	}
	// No queued movement and no current movement = stand still!
	else if (direction == Direction::NONE && queuedDirection == Direction::NONE){
		Graphics.QueueMessage(new GMSetEntity(entity, ANIMATION, "Idle"));
	}
	/// Walk more if we had stuff remaining, yo.
	if (timeRemainingAfterReachingDestination > 0){
		Process(timeRemainingAfterReachingDestination);
	}
}
