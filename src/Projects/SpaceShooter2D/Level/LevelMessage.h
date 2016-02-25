/// Emil Hedemalm
/// 2015-06-28
/// Level.

#ifndef LEVEL_MSG_H
#define LEVEL_MSG_H

#include "SpaceShooter2D/Base/Ship.h"
#include "Color.h"

class LevelMessage 
{
public:
	LevelMessage();
	void PrintAll(); // debug
	// UI Returns true if it was displayed, false if skipped (condition false).
	bool Display();
	void Hide();
	
	enum {
		TEXT_MESSAGE,
		EVENT,
	};
	String condition;
	List<String> strings;
	int type; 
	int eventType;
	enum {
		STRING_EVENT,
		GO_TO_TIME_EVENT,
	};
	bool displayed, hidden;
	Time startTime;
	Time stopTime;
	int textID;
	bool pausesGameTime; // Default true.
	Time goToTime;
};

#endif // LEVEL_MSG_H