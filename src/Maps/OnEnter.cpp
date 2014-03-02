// Emil
// What?

#include "Map.h"
#include "Event/Event.h"
#include "Event/EventManager.h"


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
			Event * event = events[i];
			if (event->triggerCondition == Event::ON_ENTER){
				// Begin le eveunt!
	//			events[i]->OnBegin();
				Event * newEvent;
				std::cout<<"\nOnEnter event found, creating new one.";
				newEvent = new Event(*event);
				std::cout<<"\nOnEnter event found, creating new one.";
				EventMan.BeginEvent(newEvent);
			}
		}
	}
	std::cout<<"\nMap::OnEnter ended: "<<name;
};
