/// Emil Hedemalm
/// 2015-01-21
/// Level.

#include "../SpaceShooter2D.h"
#include "SpawnGroup.h"
#include "Text/TextManager.h"
#include "LevelMessage.h"
#include "File/LogFile.h"

bool Level::Load(String fromSource)
{
	source = fromSource;

	failedToSurvive = false;
	defeatedAllEnemies = true;
	gameTimePaused = false;
	/// Clear old stuff.
	ships.ClearAndDelete();
	spawnGroups.ClearAndDelete();
	SpawnGroup * group = NULL;
	messages.ClearAndDelete();

	millisecondsPerPixel = 250;
	flyTime = levelTime = Time(TimeType::MILLISECONDS_NO_CALENDER, 0); // reset lvl time.

	String sourceTxt = source + ".srl";
	music = source+".ogg";
	Vector3i goalColor;
	bool messagesPauseGameTime = true;
	bool spawnGroupsPauseGameTime = true;
	
	Time startTime;

	List<ShipColorCoding> colorCodings;
	List<String> lines = File::GetLines(sourceTxt);
	enum {
		PARSE_MODE_INIT,
		PARSE_MODE_FORMATIONS,
		PARSE_MODE_MESSAGES,
	};
	int parseMode = 0;
	SpawnGroup * lastGroup = NULL;
	LevelMessage * message = NULL;
#define	ADD_GROUP_IF_NEEDED {if (group) { lastGroup = group; spawnGroups.Add(group);} group = NULL;}
#define	ADD_MESSAGE_IF_NEEDED {if (message) { messages.Add(message);} message = NULL;}
#define SET_GROUP_DEFAULTS { group->pausesGameTime = spawnGroupsPauseGameTime; }
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		// Formation specific parsing.
		List<String> tokens = line.Tokenize(" ()\t");
		if (tokens.Size() == 0)
			continue;
		String var, arg, arg2, parenthesisContents;
		if (tokens.Size() > 0)
			 var = tokens[0];
		if (tokens.Size() > 1)
			arg = tokens[1];
		if (tokens.Size() > 2)
			arg2 = tokens[2];
		// o.o
		if (line.StartsWith("StopLoading"))
			break;
		if (line.StartsWith("//"))
			continue;
		if (line.StartsWith("SpawnGroupsPauseGameTime"))
			spawnGroupsPauseGameTime = arg.ParseBool();
		else if (line.StartsWith("SpawnGroup"))
		{
			ADD_GROUP_IF_NEEDED;
			group = new SpawnGroup();
			SET_GROUP_DEFAULTS;
			// Parse time.
			String timeStr = line.Tokenize(" \t")[1];
			group->spawnTime.ParseFrom(timeStr);
			group->name = timeStr;
			parseMode = PARSE_MODE_FORMATIONS;
		}
		else if (line.StartsWith("MessagesPauseGameTime"))
		{
			messagesPauseGameTime = arg.ParseBool();
		}
		else if (line.StartsWith("Message"))
		{
			ADD_MESSAGE_IF_NEEDED;
			message = new LevelMessage();
			message->pausesGameTime = messagesPauseGameTime;
			if (arg.Length())
			{
				message->startTime.ParseFrom(arg);
				startTime = message->startTime;
			}
			else 
				message->startTime = startTime;
			message->stopTime = message->startTime + Time(TimeType::MILLISECONDS_NO_CALENDER, 5000); // Default 5 seconds?
			parseMode = PARSE_MODE_MESSAGES;
		}
		else if (line.StartsWith("Event"))
		{
			ADD_MESSAGE_IF_NEEDED;
			message = new LevelMessage();
			message->type = LevelMessage::EVENT;
			if (arg.Length())
			{
				message->startTime.ParseFrom(arg);
				startTime = message->startTime;
			}
			else
				message->startTime = startTime;
			message->stopTime = Time::Milliseconds(0); // Default instant.
			parseMode = PARSE_MODE_MESSAGES;
		}
		if (parseMode == PARSE_MODE_MESSAGES)
		{
			if (var == "Condition")
				message->condition = arg;
			if (var == "TextID")
				message->textID = arg.ParseInt();
			if (var == "String")
			{
				String strArg = line - "String";
				strArg.RemoveSurroundingWhitespaces();
				message->strings.AddItem(strArg);
			}
			if (var == "GoToTime")
			{
				message->eventType = LevelMessage::GO_TO_TIME_EVENT;
				String strArg = line - "GoToTime";
				strArg.RemoveSurroundingWhitespaces();
				message->goToTime.ParseFrom(strArg);
			}
		}
		if (parseMode == PARSE_MODE_FORMATIONS)
		{
			// Grab parenthesis
			tokens = line.Tokenize("()");
			if (tokens.Size() > 1)
				parenthesisContents = tokens[1];
			if (var == "CopyGroup")
			{
				ADD_GROUP_IF_NEEDED;
				// Copy last one.
				group = new SpawnGroup(*lastGroup);
				SET_GROUP_DEFAULTS;
				// Parse time.
				String timeStr = line.Tokenize(" \t")[1];
				group->spawnTime.ParseFrom(timeStr);
				group->name = timeStr;
				parseMode = PARSE_MODE_FORMATIONS;
			}
			if (var == "CopyNamedGroup")
			{
				ADD_GROUP_IF_NEEDED;
				// Copy last one.
				SpawnGroup * named = 0;
				List<String> tokens = line.Tokenize(" \t");
				assert(tokens.Size() >= 3);
				if (tokens.Size() < 3)
				{
					LogMain("Lacking arguments in CopyNamedGroup at line "+String(i)+" in file "+fromSource, ERROR);
					continue;
				}
				String name = tokens[1];
				for (int j = 0; j < spawnGroups.Size(); ++j)
				{
					SpawnGroup * sg = spawnGroups[j];
					if (sg->name == name)
					{
						named = sg;
						break;
					}
				}
				assert(named);
				if (!named)
				{
					LogMain("Unable to find group by name "+name+" for CopyNamedGroup at line "+String(i)+" in file "+fromSource, ERROR);
					continue;
				}
				group = new SpawnGroup(*named);
				SET_GROUP_DEFAULTS;
				// Parse time.
				String timeStr = tokens[2];
				group->spawnTime.ParseFrom(timeStr);
				group->name = timeStr;
				parseMode = PARSE_MODE_FORMATIONS;
			}
			if (var == "Name")
				group->name = arg;
			if (var == "SpawnTime")
				group->spawnTime.ParseFrom(arg);
			if (var == "Position")
				group->groupPosition.ParseFrom(parenthesisContents);
			if (var == "ShipType")
				group->shipType = arg;
			if (var == "Formation")
				group->ParseFormation(arg);
			if (var == "Number" || var == "Amount")
				group->number = arg.ParseInt();
			if (var == "Size")
				group->size.ParseFrom(parenthesisContents);
			continue;
		}
		if (line.StartsWith("MillisecondsPerPixel"))
		{
			List<String> tokens = line.Tokenize(" ");
			if (tokens.Size() < 2)
				continue;
			millisecondsPerPixel = tokens[1].ParseInt();
			if (millisecondsPerPixel == 0)
				millisecondsPerPixel = 250;
		}
		if (line.StartsWith("ShipType"))
		{
			List<String> tokens = line.Tokenize(" ");
			ShipColorCoding newCode;
			if (tokens.Size() < 2)
				continue;
			newCode.ship = tokens[1];
			assert(tokens[2] == "RGB");
			if (tokens.Size() < 6)
			{
				std::cout<<"ERrror";
				continue;
			}
			newCode.color[0] = tokens[3].ParseInt();
			newCode.color[1] = tokens[4].ParseInt();
			newCode.color[2] = tokens[5].ParseInt();
			colorCodings.Add(newCode);
		}
		else if (line.StartsWith("Goal"))
		{
			List<String> tokens = line.Tokenize(" ");
			if (tokens.Size() < 5){std::cout<<"\nError"; continue;}
			goalColor[0] = tokens[2].ParseInt();
			goalColor[1] = tokens[3].ParseInt();
			goalColor[2] = tokens[4].ParseInt();
		}
		else if (line.StartsWith("StarSpeed"))
		{
			String vector = line - "StarSpeed";
			starSpeed.ParseFrom(vector);
		}
		else if (line.StartsWith("StarColor"))
		{
			String vector = line - "StarColor";
			starColor.ParseFrom(vector);
		}
	}
	// Add last group, if needed.
	ADD_GROUP_IF_NEEDED;
	ADD_MESSAGE_IF_NEEDED;

	/// Sort groups based on spawn-time?

	// Sort messages based on time?

	// No gravity
	PhysicsMan.QueueMessage(new PMSet(PT_GRAVITY, Vector3f(0,0,0)));

	
	for (int i = 0; i < messages.Size(); ++i)
	{
	//	messages[i]->PrintAll();
	}

	// Add player? - Done later via script
	return true;
}

