/// Emil Hedemalm
/// 2015-05-18
/// Code for the Paco-Taco mini-game events!

#include "SideScroller.h"

/// Update ui
void OnRPSUpdated();
void UpdateCountdown(int toNumber = -1);
void ShowCountdown();
void HideCountdown();
void UpdateKeyToSpam();
void UpdateKeyToStrike(); // For pose. Might become a combination?
void UpdateTextToEnter();
void SetPTState(int newState);

void PlaceBit();

List<String> bits; // source for textures.
Random pacoRand;
enum 
{
	RockPaperScissors,
	BuildATaco,
	SpamEatTacos,
	StrikeAPose, PACO_TACO_EVENT_TYPES
};
int which = RockPaperScissors;
Time timeStarted;
Time timeToPose;
int msPassed;
int countdownTimer;
int roundsDone;
int playerWins, pacoWins;
int rps, pacoRPS;
int spammed;
int keyToSpam;
int keyToStrike;
int bitsPlaced;
bool badTime;
String textToEnter;
enum 
{
	ROCK, PAPER, SCISSORS
};
enum 
{
	INTRODUCTION,
	PREPARING,
	ACTIVE,
	RESULT,
	REWARD,
	ENDING,
};
int ptState = INTRODUCTION;

/// Called per frame while in the event.
void SideScroller::ProcessPacoTaco(int timeInMs)
{
	if (paused)
		return;
	msPassed += timeInMs;
	if (msPassed > 1000 && countdownTimer > 0)
	{
		msPassed = 0;
		countdownTimer -= 1;
		UpdateCountdown();
		if (countdownTimer == 0)
		{
			// Default 5 seconds.
			SetPTState(ptState + 1);
		}
	}
}

#define SetRPS(ui,s) QueueGraphics(new GMSetUIs(ui, GMUI::TEXTURE_SOURCE, String("img/PacoTaco/RockPaperScissors/")+(s)+String(".png")))
#define SetPlayerRPS(s) SetRPS("RPSRight", String("Luchador_hand_")+(s))
#define SetPacoRPS(s) SetRPS("RPSLeft", String("Paco_hand_")+(s))

void SetPTState(int newState)
{
	countdownTimer = 5;
	int oldState = ptState;
	ptState = newState;
	/// Which state are we leaving?
	switch(oldState)
	{
		case INTRODUCTION:
			// Hide help text!
			QueueGraphics(new GMSetUIb("PacoTacoHelpText", GMUI::VISIBILITY, false));
			if (which == BuildATaco)
			{
				QueueGraphics(new GMClearUI("BATTaco"));
			}
			break;
		case RESULT:
			if (which == RockPaperScissors)
			{
				// Game over.
				if (playerWins >= 3 || pacoWins >= 3)
				{
					// Show final result?
					if (playerWins > pacoWins)
					{
						QueueGraphics(new GMSetUIs("rpsResult", GMUI::TEXT, TextMan.GetText(1051)));
					}
					else 
						QueueGraphics(new GMSetUIs("rpsResult", GMUI::TEXT, TextMan.GetText(1052)));

				}
				// Next round.
				else 
				{
					SetPTState(ACTIVE);
					ShowCountdown();
					SetPlayerRPS("passive");
					SetPacoRPS("passive");
				}
			}
			break;
	}
	/// Which state are we entering?
	switch(ptState)
	{
		case INTRODUCTION:
		{
			HideCountdown();
			QueueGraphics(new GMSetUIb("PacoTacoHelpText", GMUI::VISIBILITY, true));
			switch(which)
			{
				case RockPaperScissors:
					QueueGraphics(new GMSetUIb("RPSIntroImage", GMUI::VISIBILITY, true));
					QueueGraphics(new GMSetUIb("RPSLeft", GMUI::VISIBILITY, false));
					QueueGraphics(new GMSetUIb("RPSRight", GMUI::VISIBILITY, false));

					break;
			}
			break;	
		}
		case PREPARING:
			ShowCountdown();
			countdownTimer = 5; // Countdown again?
			QueueGraphics(new GMSetUIb("RPSIntroImage", GMUI::VISIBILITY, false));
			badTime = true;
			switch(which)
			{
				case RockPaperScissors:
					SetPTState(ACTIVE); // Skip preparation step. Active is where we choose.
					break;
				case BuildATaco:
					// Generate text.
					for (int i = 0; i < 20; ++i)
					{
						int v = KEY::A;
						// Vowel?
						if (i % 4 == 1)
						{
							int v = pacoRand.Randi(5);
							switch(v)
							{
							case 0:	v = KEY::A; break;
							case 1: v = KEY::E; break;
							case 2: v = KEY::I; break;
							case 3: v = KEY::O; break;
							case 4: v = KEY::U; break;
							case 5: v = KEY::Y; break;
							}
						}
						// Any letter
						else
							v = pacoRand.Randi(KEY::Z - KEY::A) + KEY::A;
						textToEnter += GetKeyString(v);
					}
//					textToEnter = "PARAPARAPARAPARA";
					UpdateTextToEnter();
					countdownTimer = 2;
					break;
			}
			break;
		case ACTIVE:
			countdownTimer = 15;
			switch(which)
			{
				case RockPaperScissors:
					QueueGraphics(new GMSetUIb("RPSLeft", GMUI::VISIBILITY, true));
					QueueGraphics(new GMSetUIb("RPSRight", GMUI::VISIBILITY, true));
					countdownTimer = 5;
					break;
				case StrikeAPose:
					timeToPose = now;
					badTime = false;
					HideCountdown();
					countdownTimer = 5;
					break;
			}
			break;
		case RESULT:
			countdownTimer = 3;
			HideCountdown();
			switch(which)
			{
				case RockPaperScissors:
				{
					// Select for AI.
					pacoRPS = pacoRand.Randi(2);
					// Update graphics.
					
					SetPlayerRPS(rps == ROCK ? "rock" : rps == PAPER? "paper" : "scissors");
					SetPacoRPS(pacoRPS == ROCK ? "rock" : pacoRPS == PAPER? "paper" : "scissors");
					
					int t = 0;
					if (rps == pacoRPS)
					{
						// Draw
						t = 1041;
					}
					else if ((rps == ROCK && pacoRPS == SCISSORS) || 
						(rps == PAPER && pacoRPS == ROCK) ||
						(rps == SCISSORS && pacoRPS == PAPER))
					{
						// Player win
						t = 1042;
						++playerWins;
					}
					else 
					{
						t = 1043;
						++pacoWins;
					}
					QueueGraphics(new GMSetUIs("rpsResult", GMUI::TEXT, TextMan.GetText(t)));
					++roundsDone;
				}
			}
			break;
		case REWARD:
			// Hide countdown?
			HideCountdown();
			countdownTimer = 2;
			break;
		case ENDING:
			// Hide countdown?
			HideCountdown();
			countdownTimer = 1;
			sideScroller->ReturnToPreviousState();
			break;
	}
	UpdateCountdown();
}

/// Callback from the Input-manager, query it for additional information as needed.
void SideScroller::KeyPressedPacoTaco(int keyCode, bool downBefore)
{
	if (ptState != ACTIVE)
		return;

	switch(which)
	{
		case RockPaperScissors:
			// Select based on [R], [P], [S] keys.
			switch(keyCode)
			{
			case KEY::R:
			case KEY::ONE:
				rps = ROCK;
				break;
			case KEY::P:
			case KEY::TWO:
				rps = PAPER;
				break;
			case KEY::S:
			case KEY::THREE:
				rps = SCISSORS;
				break;
			}
			OnRPSUpdated();
			break;
		case SpamEatTacos:
			// Don't accept OS spam. Require user to re-press.
			if (keyCode == keyToSpam)
			{
				// Accept all spam? Or one key at a time?
				++spammed;
				QueueGraphics(new GMSetUIs("SpamCount", GMUI::TEXT, String(spammed)));
				/// Change if they be spamming. Also change every 10 key-strikes?
				if (downBefore || spammed % 10 == 0)
					UpdateKeyToSpam();
			}
			break;
		case BuildATaco:
		{
			char c = textToEnter.At(0);
			String keyStr = GetKeyString(keyCode);
			if (keyStr.At(0) == c)
			{
				PlaceBit();
				// Another!
				if (bitsPlaced > 10)
					PlaceBit();
				if (bitsPlaced > 20)
					PlaceBit();
				textToEnter = textToEnter.Part(1);
				UpdateTextToEnter();
			}
			if (textToEnter.Length() == 0)
			{
				// Win! o.o
				SetPTState(RESULT);
			}
			break;
		}
		case StrikeAPose:
		{
			if (keyCode != keyToStrike)
				return;
			// Check timing.
			if (badTime)
			{
				/// Fail!
				return;
			}
			// Take note of time.
			int ms = (now - timeToPose).Milliseconds();
			int textID;
			if (ms < 20)
			{
				textID = 1031;
			}
			else if (ms < 50)
			{
				textID = 1032;					
			}
			else if (ms < 90)
			{
				textID = 1033;
			}
			else if (ms < 150)
			{
				textID = 1034;				
			}
			else if (ms < 500)
			{
				textID = 1035;			
			}
			else 
			{
				textID = 1036;
			}
			QueueGraphics(new GMSetUIs("StrikeAResult", GMUI::TEXT, TextMan.GetText(textID)));
			QueueGraphics(new GMSetUIb("StrikeAResult", GMUI::VISIBILITY, true));
			break;	
		}
	}
}

void SideScroller::ProcessMessagePacoTaco(Message * message)
{
	
}

/// Called to start the event.
void SideScroller::InitiatePacoTaco()
{
	spammed = 0;
	bitsPlaced = 0;
	bits.Clear(); 
	String bitsDir = "img/PacoTaco/Bits/";
	bool ok = GetFilesInDirectory(bitsDir, bits);
	PrependStrings(bits, bitsDir);
	assert(ok);
	msPassed = 0;
	roundsDone = 0;
	playerWins = pacoWins = 0;
	countdownTimer = 5;
	timeStarted = Time::Now();
	which = pacoRand.Randi(1000) % PACO_TACO_EVENT_TYPES;
	SetState(PACO_TACO, true);
	SetPTState(INTRODUCTION);
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
	QueueGraphics(new GMSetUIs("rpsSelected", GMUI::TEXT, ""));
	QueueGraphics(new GMSetUIs("rpsResult", GMUI::TEXT, ""));
//	QueueGraphics(new GMSetUIb("PacoTacoCountdown", GMUI::VISIBILITY, true)); // Make countdown timer visible?
	QueueGraphics(new GMSetUIb("StrikeAResult", GMUI::VISIBILITY, false));
	UpdateKeyToSpam();
	UpdateKeyToStrike();
	UpdateTextToEnter();
}

void OnRPSUpdated()
{
	// Notify
	QueueGraphics(new GMSetUIs("rpsSelected", GMUI::TEXT, 
		rps == ROCK? TextMan.GetText(1021) : rps == PAPER? TextMan.GetText(1022) : TextMan.GetText(1023)));
}

void UpdateCountdown(int toNumber /*= -1*/)
{
	// countdown = toNumber;
	QueueGraphics(new GMSetUIs("PacoTacoCountdown", GMUI::TEXT, String(countdownTimer)));
}
void ShowCountdown()
{
	QueueGraphics(new GMSetUIb("PacoTacoCountdown", GMUI::VISIBILITY, true));
}
void HideCountdown()
{
	QueueGraphics(new GMSetUIb("PacoTacoCountdown", GMUI::VISIBILITY, false));
}
void UpdateKeyToSpam()
{
	keyToSpam = KEY::A + pacoRand.Randi(KEY::Z - KEY::A);
	QueueGraphics(new GMSetUIs("KeyToSpam", GMUI::TEXT, GetKeyString(keyToSpam)));
}

void UpdateKeyToStrike()
{
	keyToStrike = KEY::A + pacoRand.Randi(KEY::Z - KEY::A);
	QueueGraphics(new GMSetUIs("KeyToStrike", GMUI::TEXT, GetKeyString(keyToStrike)));
}

void UpdateTextToEnter()
{
	QueueGraphics(new GMSetUIs("keysToEnter", GMUI::TEXT, textToEnter));
}

void PlaceBit()
{
	/// Add UI representing the veggie! :3
	UIImage * image = new UIImage("Veggie X");
	image->textureSource = bits[pacoRand.Randi() % bits.Size()];
	image->sizeRatioX = image->sizeRatioY = 0.125f + bitsPlaced * 0.002f + pacoRand.Randf(0.05f);
	++bitsPlaced;

	// Find line to place along.
	Vector2f start(0.265f, 0.18f);
	Vector2f end(0.58f, 0.9f);
	Vector2f righter = Vector2f(0.02f, 0);
	start += righter;
	end += righter;
	Vector2f startToEnd = end - start;
	Vector2f middle = start + startToEnd * 0.5f;

	Vector2f pos = start + startToEnd * pacoRand.Randf();
	// Randomize closeness to center so we get fewer bits along the sides?
	float middleness = pacoRand.Randf(0.5f);
	pos = pos * (1 - middleness) + middle * middleness;
	image->alignmentX = pos.x + pacoRand.Randf(0.1f) - 0.05f; // Add some variance.
	image->alignmentY = pos.y;

	QueueGraphics(new GMAddUI(image, "BATTaco"));
}