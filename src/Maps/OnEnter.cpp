// Emil
// What?

#include "Map.h"
#include "Script/Script.h"
#include "Script/ScriptManager.h"


struct Map::OnEnterAttributes{
	const char * musicAtZone;
};

// Wosh? o.o
void Map::OnEnter(){

	std::cout<<"\nMap::OnEnter map: "<<name;
	if (events.Size()){
		std::cout<<"\nLoadeding events..";
		bool result = LoadEvents();
		std::cout<<"\nLoaded events: "<<result;
		for (int i = 0; i < events.Size(); ++i){
			Script * event = events[i];
			if (event->triggerCondition == Script::ON_ENTER){
				// Begin le eveunt!
	//			events[i]->OnBegin();
				Script * newEvent;
				std::cout<<"\nOnEnter event found, creating new one.";
				newEvent = new Script(*event);
				std::cout<<"\nOnEnter event found, creating new one.";
				ScriptMan.BeginEvent(newEvent);
			}
		}
	}
	std::cout<<"\nMap::OnEnter ended: "<<name;
};
