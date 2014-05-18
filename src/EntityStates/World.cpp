//// Emil Hedemalm
//// Deprecated until further notice.
//
#include "World.h"
//
//
//#include "Locations/GenericLocation.h"
//#include "Locations/Locations.h"
//#include "LocationStates/LocationState.h"
//#include "AIMessages.h"
//
//World World::world;
//
///// PRivate constructor
//World::World(){
//};
///// Destructor
//World::~World(){
//};
//
///// Posts a message to the world, re-directed depending on message-type.
//void World::Message(AIMessage * message){
//	/// World messages
//	if (message->target == NULL){
//		switch(message->type){
//			case AIMsg::CHARACTER_DEAD:{
//				Entity * e = message->sender;
//				std::cout<<"\nWORLD: A terrible tragedy has befallen us all! "<<e->name<<" has died!";
//				/// Remove characters
//				for (int i = 0; i < MAX_CHARACTERS; ++i){
//					if (character[i] == e){
//						character[i] = NULL;
//					}
//				}
//				AI.UnregisterEntity(e);
//				Physics.QueueMessage(new PMUnregisterEntity(e));
//				Graphics.QueueMessage(new GMUnregisterEntity(e));
//				((CharacterState*)e->state->globalState)->alive = false;
//				break;
//			}
//			case AIMsg::SOCIALIZE_ATLOCATION: {
//				/// Inform all entities going to target location that a friend is now there ^w^
//				for (int i = 0; i < characters; ++i){
//					if (!character[i])
//						continue;
//					if (((CharacterState*)character[i]->state->globalState)->Destination() == message->data){
//						if (character[i]->state->currentState->stateID == CharState::SOCIALIZING){
//							++((Socializing*)character[i]->state->currentState)->friendsAtLocation;
//						}
//					}
//				}
//				break;
//			}
//			case AIMsg::SOCIALIZE_BYE:{
//				/// Inform all entities going to target location that a friend is now there ^w^
//				for (int i = 0; i < characters; ++i){
//					if (!character[i])
//						continue;
//					if (((CharacterState*)character[i]->state->globalState)->Destination() == message->data){
//						if (character[i]->state->currentState->stateID == CharState::SOCIALIZING){
//							--((Socializing*)character[i]->state->currentState)->friendsAtLocation;
//						}
//					}
//				}
//				break;
//			}
//		}
//	}
//	/// Entity messages
//	else {
//		message->target->state->currentState->onMessage(message);
//		message->target->state->globalState->onMessage(message);
//	}
//	delete message;
//};
//
///// Returns amount of characters at target location.
//int World::CharactersAtLocation(Location * location){
//	int res = 0;
//	for (int i = 0; i < MAX_CHARACTERS; ++i){
//		if (character[i] && ((CharacterState*)character[i]->state->globalState)->currentLocation == location)
//			++res;
//	}
//	return res;
//}
//
//bool World::AddLocation(Entity * location){
//	for (int i = 0; i < MAX_LOCATIONS; ++i){
//		if (!locList[i]){
//			locList[i] = location;
//			return true;
//		}
//	}
//	return false;
//}
//bool World::AddCharacter(Entity * i_character){
//	for (int i = 0; i < MAX_CHARACTERS; ++i){
//		if (!character[i]){
//			character[i] = i_character;
//			++characters;
//			return true;
//		}
//	}
//	return false;
//}
//
//int World::Characters(){
//	int chars = 0;
//	for (int i = 0; i < characters; ++i){
//		if (character[i]){
//			++chars;
//		}
//	}
//	return chars;
//};
//
///// Returns entity at target index
//Entity * World::GetCharacter(int index){
//	int searchIndex = 0;
//	for (int i = 0; i < characters; ++i){
//		if (character[i]){
//			if (searchIndex == index)
//				return character[i];
//			++searchIndex;
//		}
//	}
//	return NULL;
//};
//
//void World::CreateDefaultSetup(){
//	/// Create locations
//#define GetPlane	ModelMan.GetModel("plane.obj")
//	/// Load textures..!
//	TexMan.LoadTexture("img/AITest/mineshaft.png");
//	TexMan.LoadTexture("img/AITest/shop.png");
//	TexMan.LoadTexture("img/AITest/ironworks.png");
//	TexMan.LoadTexture("img/AITest/residence.png");
//	TexMan.LoadTexture("img/AITest/park.png");
////	TexMan.LoadTexture("img/AITest/mineshaft.png");
//
//	/// Characters to create!
//	int characters = 4;
//
//
//#define DEFAULT_SCALE 15
//#define DEFAULT_RADIUS (DEFAULT_SCALE /2.0f)
//
///// Mineshaft o-o;
//	Entity * location = MapMan.CreateEntity(GetPlane, TexMan.GetTextureBySource"img/AITest/mineshaft.png"));
//	location->state = new StateProperty();
//	/// Create location state
//	LocationState * ls =  new LocationState();
//		/// Define workplace attributes
//		Workplace * wp = new Workplace();
//			wp->coinsPerHour = 5;
//			wp->location = Vector3f(40,1,20);
//			strcpy(wp->name, "The Mines");
//			wp->locationEntity = location;
//			wp->radius = DEFAULT_RADIUS;
//		ls->location = wp;
//	location->state->globalState = ls;
//	location->position(40, 0, 20);
//	location->Scale(DEFAULT_SCALE);
//	AddLocation(location);
//
///// Shop ^.^
//	location = MapMan.CreateEntity(GetPlane, TexMan.GetTextureBySource"img/AITest/shop.png"));
//	location->position(10, 0, 15);
//	location->Scale(DEFAULT_SCALE);
//	location->state = new StateProperty();
//	/// Create location state
//	ls =  new LocationState();
//		/// Define workplace attributes
//		Shop * shop = new Shop();
//			shop->location = location->position + Vector3f(0,1,0);
//			strcpy(shop->name, "Ye Olde Shop");
//			shop->locationEntity = location;
//			shop->radius = DEFAULT_RADIUS;
//			shop->sellableItems[0] = (Item*) new Food("Pork Stew", 15, 0.00012f);
//			shop->sellableItems[1] = (Item*) new Drink("Pale Ale", 5, 0.0006f);
//			shop->sellableItems[2] = (Item*) new Tool("Shovel", 115, 1.1f);
//			shop->sellableItems[3] = (Item*) new Vehicle("Horse Carriage", 575, 1.1f);
//		ls->location = shop;
//	location->state->globalState = ls;
//	AddLocation(location);
//
///// Ironworks ^^
//	location = MapMan.CreateEntity(GetPlane, TexMan.GetTextureBySource"img/AITest/ironworks.png"));
//	location->position(30, 0, -15);
//	location->Scale(DEFAULT_SCALE * 2);
//	location->state = new StateProperty();
//	/// Create location state
//	ls =  new LocationState();
//		/// Define workplace attributes
//		wp = new Workplace();
//			wp->coinsPerHour = 4;
//			wp->location = location->position + Vector3f(0,1,0);
//			strcpy(wp->name, "Ironworks");
//			wp->locationEntity = location;
//			wp->radius = DEFAULT_RADIUS;
//		ls->location = wp;
//	location->state->globalState = ls;
//	AddLocation(location);
//
///// Homes ^-^
//#define HOUSE_SCALE DEFAULT_SCALE
//	/// Get random seed
//	srand((unsigned int)time(NULL));
//	for (int i = 0; i < characters; ++i){
//		location = MapMan.CreateEntity(GetPlane, TexMan.GetTextureBySource"img/AITest/residence.png"));
//		location->position(-50.0f, 0, -50.0f + (100.0f / characters) * i);
//		location->Scale(HOUSE_SCALE);
//		location->state = new StateProperty();
//		/// Create location state
//		ls =  new LocationState();
//			/// Define residence attributes
//			Residence * r = new Residence();
//				r->price = 50 + rand()%50;
//				r->location = location->position + Vector3f(0,1,0);
//				strcpy(r->name, "Residence ");
//				char buf[4]; memset(buf, 0, 4);
//				_itoa(i+1, buf, 10);
//				strcat(r->name, buf);	/// Append number to the residence for clarity~
//				r->locationEntity = location;
//				r->radius = DEFAULT_RADIUS;
//			ls->location = r;
//		location->state->globalState = ls;
//		AddLocation(location);
//	}
//
//
///// Park ^3^
//	location = MapMan.CreateEntity(GetPlane, TexMan.GetTextureBySource"img/AITest/park.png"));
//	location->position(-15, 0, -25);
//	location->Scale(DEFAULT_SCALE);
//	location->state = new StateProperty();
//	/// Create location state
//	ls =  new LocationState();
//		/// Define workplace attributes
//		GenericLocation * g = new GenericLocation();
//			g->socialFactor = 2.0f;
//			g->location = location->position + Vector3f(0,1,0);
//			strcpy(g->name, "Park");
//			g->locationEntity = location;
//			g->radius = DEFAULT_RADIUS;
//		ls->location = g;
//	location->state->globalState = ls;
//	AddLocation(location);
//
//	// Set all locations as planes ^^
//	Physics.QueueMessage(new PMSetPhysicsShape(MapMan.GetEntities(), ShapeType::PLANE));
//
///// Create ze charucturs! =^w^=
//
//	/// Set it to ignore collissions (for now)
//	Physics.QueueMessage(new PhysicsMessage(PM_IGNORE_COLLISSIONS));
//
//	TexMan.LoadTexture("img/logo.png");
//	/// Get random seed
//	srand((unsigned int)time(NULL));
//	for (int i = 0; i < characters; ++i){
//		Entity * character = MapMan.CreateEntity(ModelMan.GetModel("sphere6.obj"), TexMan.GetTextureBySource"img/logo.png"));
//		character->position(-50.0f + rand()%100, 1.0f, -50.0f + rand()%100);
//		character->Scale(1);
//		character->state = new StateProperty();
//		char buf[4];
//		String name;
//		switch(i){
//			case 0: name = "Karl"; break;
//			case 1: name = "Born"; break;
//			case 2: name, "Liam"; break;
//			case 3: name, "Olid"; break;
//			default:
//				name = "Char ";
//				_itoa(i+1, buf, 10);
//				name += buf;
//		}
//		character->name = name;
//		/// Assign default residences and workplaces
//		int work = rand()%3;
//		Entity * toLive = locList[3 + i];
//
//		/// Create it's global CharacterState
//		CharacterState * cs =  new CharacterState();
//		character->state->globalState = cs;
//		cs->entity = character;
//		cs->currentWork = this->GetWorkplace();
//		cs->currentHome = (Residence*)((LocationState*)toLive->state->globalState)->location;
//
//		/// And an initial state for when entering the game..!
//		character->state->EnterState(new Working(), character);
//		/// Add it to our world's list.
//		AddCharacter(character);
//		/// Register it with the AIManager
//		AI.RegisterEntity(character);
//		/// Set type to dynamic
//		Physics.QueueMessage(new PMSetPhysicsType(List<Entity*>(character), PhysicsType::DYNAMIC));
//	}
//}
//
//
///// Re-places all locations to valid waypoints.
//void World::MapToNavMesh(){
//
//	for (int i = 0; i < this->characters; ++i){
//		if (this->character[i] == NULL){
//			std::cout<<"World::MapToNavMesh: Null-character in what should be a compact array! Character is presumed dead";
//			continue;
//		}
//		CharacterState * cs = ((CharacterState*)character[i]->state->globalState);
//		Waypoint * wp = WaypointMan.GetClosestValidWaypoint(character[i]->position);
//		cs->currentLocWp = wp;
//	}
//	for (int i = 0; i < this->MAX_LOCATIONS; ++i){
//		if (locList[i] == NULL)
//			continue;
//		Waypoint * wp = NULL;
//		wp = WaypointMan.GetClosestValidFreeWaypoint(locList[i]->position);
//		assert(wp->passable && "WaypointMan.GetClosestValidFreeWaypoint returned an unpassable tile! D:");
//		LocationState * ls = ((LocationState*)locList[i]->state->globalState);
//		if (!wp){
//			assert(wp->pData == NULL && "Error in World::MapToNavMesh. There are fewer waypoints than there are locations! Some locations will not get a waypoint bound to it.");
//			ls->location->wp = NULL;
//		}
//		if(wp->pData != NULL){
//			std::cout<<"\nERROR: Waypoint already has data bound to it!";
//			assert(wp->pData == NULL && "Error in World::MapToNavMesh. There are fewer waypoints than there are locations! Some locations will not get a waypoint bound to it.");
//			ls->location->wp = NULL;
//		};
//		wp->pData = locList[i];
//		ls->location->wp = wp;
//		ls->location->location = wp->position;
//		locList[i]->position = wp->position;
//		locList[i]->RecalculateMatrix();
//
//	}
//}
//
