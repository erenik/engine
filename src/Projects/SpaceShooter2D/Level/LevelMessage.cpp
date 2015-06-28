/// Emil Hedemalm
/// 2015-06-28
/// Level.

#include "SpawnGroup.h"
#include "Text/TextManager.h"
#include "../SpaceShooter2D.h"
#include "LevelMessage.h"
#include "File/LogFile.h"

LevelMessage::LevelMessage()
{
	displayed = hidden = false;
	goToTime = startTime = stopTime = Time(TimeType::MILLISECONDS_NO_CALENDER, 0);
	type = TEXT_MESSAGE;
	eventType = STRING_EVENT;
	pausesGameTime = false;
	textID = -1;
}

void LevelMessage::PrintAll()
{
	std::cout<<"\nType: "<<(type == TEXT_MESSAGE? "Text message" : "Event");
	std::cout<<"\nTime: "<<startTime.Milliseconds();
	if (type == TEXT_MESSAGE)
	{
		std::cout<<"\nTextID: "<<textID;
	}
}


int activeLevelDisplayMessages = 0;

// UI
bool LevelMessage::Display()
{
	bool trigger = true;
	if (condition.Length())
	{
		std::cout<<"\nCondition: "<<condition;
		if (condition == "FailedToDefeatAllEnemies")
		{
			trigger = defeatedAllEnemies == false;
		}
		else if (condition == "FailedToSurvive")
		{
			trigger = failedToSurvive;
		}
		else {
			LogMain("Bad condition in LevelMessage", WARNING);
		}
		std::cout<<" trigger: "<<trigger;
	}
	if (!trigger)
	{
		// Mark it as if it has been displayed and triggered already?
		displayed = hidden = true;
		return false;
	}


	if (type == LevelMessage::TEXT_MESSAGE)
	{
		// o.o uiiii
		if (textID == -1)
		{
			PrintAll();
		}
		QueueGraphics(new GMSetUIs("LevelMessage", GMUI::TEXT, TextMan.GetText(textID)));
		QueueGraphics(new GMSetUIb("LevelMessage", GMUI::VISIBILITY, true));
		displayed = true;
		++activeLevelDisplayMessages;
	}
	else if (type == LevelMessage::EVENT)
	{
		displayed = true;
		if (strings.Size())
			MesMan.QueueMessages(strings);
		if (goToTime.intervals != 0)
		{
			spaceShooter->level.SetTime(goToTime);
			LogMain("Jumping to time: "+String(goToTime.Seconds()), INFO);
			return false; // Return as if it failed, so the event is not saved as currently active one. (instantaneous)
		}
	}
	if (pausesGameTime)
	{
		gameTimePaused = true;
		// If any entities are present, pause game entirely?
	}
	return true;
}

void LevelMessage::Hide()
{
	if (type == LevelMessage::TEXT_MESSAGE)
	{
		--activeLevelDisplayMessages;
		if (activeLevelDisplayMessages <= 0)
		{
			QueueGraphics(new GMSetUIb("LevelMessage", GMUI::VISIBILITY, false));
			gameTimePaused = false;
		}
		displayed = true;
	}
	hidden = true;
	if (pausesGameTime)
		gameTimePaused = false;
}
