/// Emil Hedemalm
/// 2015-05-18
/// Code for the Paco-Taco mini-game events!

#include "SideScroller.h"

enum 
{
	RockPaperScissors,
	BuildATaco,
	SpamEatTacos,
	StrikeAPose, PACO_TACO_EVENT_TYPES
};
int which = RockPaperScissors;
Time timeStarted;
int msPassed = 0;
int countdownTimer = 5;
enum 
{
	INTRODUCTION,
	PREPARING,
	ACTIVE,
	ENDING,
};
int ptState = INTRODUCTION;

/// Called per frame while in the event.
void SideScroller::ProcessPacoTaco(int timeInMs)
{
	msPassed += timeInMs;
	if (msPassed > 1000 && countdownTimer > 0)
	{
		countdownTimer -= 1;
		QueueGraphics(new GMSetUIs("PacoTacoCountdown", GMUI::TEXT, String(countdownTimer)));
		if (countdownTimer == 0)
		{
			++ptState;
		}
	}
}

/// Callback from the Input-manager, query it for additional information as needed.
void SideScroller::KeyPressedPacoTaco(int keyCode, bool downBefore)
{
	if (ptState < ACTIVE)
		return;
	switch(which)
	{
		case RockPaperScissors:
			// Select based on [R], [P], [S] keys.

			break;
		case SpamEatTacos:
			// Accept all spam? Or one key at a time?

			break;
	}
}

void SideScroller::ProcessMessagePacoTaco(Message * message)
{
	
}

/// Called to start the event.
void SideScroller::InitiatePacoTaco()
{
	ptState = INTRODUCTION;
	msPassed = 0;
	timeStarted = Time::Now();
	Random pacoRand;
	which = pacoRand.Randi(1000) % PACO_TACO_EVENT_TYPES;
	SetState(PACO_TACO, true);
}

/// Called to end the event.
void SideScroller::EndPacoTaco()
{
	MesMan.QueueMessages("Run");
}

void SideScroller::UpdatePacoTacoUI()
{
	String a = "RockPaperScissors", b = "BuildATaco", c = "SpamEatTacos", d = "StrikeAPose";
	List<String> l;
	l.Add(a,b,c,d);
	PopUI(a);
	PopUI(b);
	PopUI(c);
	PopUI(d);
	String toPush = l[which];
	PushUI(toPush);
	String s, h;
	switch(which)
	{
		case RockPaperScissors: 
			s = TextMan.GetText(1011);
			h = TextMan.GetHelpText(1001);
			break;
		case BuildATaco: 
			s = TextMan.GetText(1012);
			h = TextMan.GetHelpText(1002);
			break;
		case SpamEatTacos: 
			s = TextMan.GetText(1013);
			h = TextMan.GetHelpText(1003);
			break;
		case StrikeAPose: 
			s = TextMan.GetText(1014);
			h = TextMan.GetHelpText(1004);
			break;
	}
	QueueGraphics(new GMSetUIs("PacoTacoEvent", GMUI::TEXT, s));
	QueueGraphics(new GMSetUIs("PacoTacoHelpText", GMUI::TEXT, h));
	QueueGraphics(new GMSetUIb("PacoTacoCountdown", GMUI::VISIBILITY, true)); // Make countdown timer visible?
}


