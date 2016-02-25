/// Emil Hedemalm
/// 2015-07-09
/// Loader for Ship, separate file due to length.

#include "SpaceShooter2D/Base/Ship.h"
#include "File/File.h"
#include "String/StringUtil.h"
#include "File/LogFile.h"

String whenParsingFile;
int row = 0;
#define LogShip(s) LogMain(s, INFO); // "ShipParserErrors.txt", whenParsingFile + " row "+String(row+1) + String(": ") + s)
// Load ship-types.
bool Ship::LoadTypes(String file)
{
	LogMain("Loading ships from file: \""+file+"\"", INFO);
	whenParsingFile = file;
	List<String> lines = File::GetLines(file);
	if (lines.Size() == 0)
		return false;

	file.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	bool isBoss = file.Contains("boss");
	String separator;
	/// Column-names. Parse from the first line.
	List<String> columns;
	String firstLine = lines[0];
	char delimiter = FindCSVDelimiter(firstLine);;
	// Keep empty strings or all will break.
	columns = TokenizeCSV(firstLine, delimiter);

	// For each line after the first one, parse data.
	for (int j = 1; j < lines.Size(); ++j)
	{
		String & line = lines[j];
		row = j;
		// Keep empty strings or all will break.
		List<String> values = TokenizeCSV(line, delimiter);
		for (int i = 0; i < values.Size(); ++i)
		{
			String v = values[i];
	//		std::cout<<"\nValue "<<i<<": "<<v;
		}
		// If not, now loop through the words, parsing them according to the column name.
		// First create the new spell to load the data into!
		Ship * ship = new Ship();
		for (int k = 0; k < values.Size(); ++k)
		{
			String column;
			bool error = false;
			/// In-case extra data is added beyond the columns defined above..?
			if (columns.Size() > k)
				column = columns[k];
			String value = values[k];
			if (value == "n/a")
				continue;
			column.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (column == "Name")
			{
				value.RemoveSurroundingWhitespaces();
				ship->name = value;
			}
			else if (column == "Type")
				ship->type = value;
			else if (column == "Difficulty")
				ship->difficulty = value.ParseInt();			
			else if (column == "OnCollision")
				ship->onCollision = value;
			else if (column == "Weapons")
			{
				List<String> weaponNames = value.Tokenize(",");
				// Fetch weapons?
				for (int i = 0; i < weaponNames.Size(); ++i)
				{
					String name = weaponNames[i];
					name.RemoveInitialWhitespaces();
					name.RemoveTrailingWhitespaces();
					Weapon * weapon = new Weapon(); 
					bool ok = Weapon::Get(name, weapon);
					if (ok)
						ship->weapons.Add(weapon);
					else {
						LogMain("Unable to find weapon by name \'"+name+"\' when parsing ship \'"+ship->name+"\'", INFO);
						delete weapon;
					}	
				}
				if (ship->weapons.Size())
					ship->canShoot = true;
			}
			else if (column == "Weapon Locations")
			{
				List<String> locations = value.Tokenize(",");
				int numLocations = locations.Size();
				int numWeaps = ship->weapons.Size();
				if (numLocations != numWeaps)
				{
					LogMain("Bad number of locations for weapons, "+String(numLocations)+" locations for "+String(numWeaps)+" weapons", INFO);
					continue;
				}
				for (int i = 0; i < ship->weapons.Size(); ++i)
				{
					Weapon * weapon = ship->weapons[i];
					String locStr = locations[i];
					weapon->location.ParseFrom(locStr);
					std::cout<<"\nWeap "<<weapon->name<<" pos"<<weapon->location;
				}
			}
			else if (column == "Movement pattern")
			{
				ship->ParseMovement(value);
				ship->movements.Size();
				ship->canMove = true;
			}
			else if (column == "Rotation pattern")
			{
				ship->ParseRotation(value);
				ship->rotations.Size();
			}
			else if (column == "Score")
				ship->score = value.ParseInt();
			else if (column == "Speed")
				ship->speed = value.ParseFloat() * 0.2f;
			else if (column == "Children")
				ship->childrenStrings = value.Tokenize(",");
			else if (column == "Has Shield")
				ship->hasShield = value.ParseBool();
			else if (column == "Shield Value")
				ship->shieldValue = ship->maxShieldValue = (float) value.ParseInt();
			else if (column == "Shield Regen Rate")
				ship->shieldRegenRate = value.ParseInt() / 1000.f;
			else if (column == "Hit points")
				ship->maxHP = ship->hp = value.ParseInt();
			else if (column == "Collide damage")
				ship->collideDamage = value.ParseInt();
			else if (column == "Script")
				ship->scriptSource = value;
			else if (column == "Max rotation per second" || column == "Rotation Speed")
				ship->maxRadiansPerSecond = DEGREES_TO_RADIANS(value.ParseInt());
			else if (column == "Graphic model")
				ship->graphicModel = value;
			else if (column == "Physics Model")
				ship->physicsModel = value;
			else 
			{
				LogMain("Unrecognized column name \'"+column+"\'.", INFO);
			}
		}

		ship->boss = isBoss;		LogMain("Ship loaded: "+ship->name+", weapons: "+ship->weapons.Size(), INFO);
		// Check for pre-existing ship of same name, remove it if so.
		for (int i = 0; i < types.Size(); ++i)
		{
			Ship * type = types[i];
			if (type->name == ship->name)
			{
				types.RemoveIndex(i);
				--i;
			}
		}
		types.AddItem(ship);
	}
	return true;
}

/// E.g. "Straight(10), MoveTo(X Y 5 20, 5)"
void Ship::ParseMovement(String fromString)
{
	movements.Clear();
	/// Tokenize.
	List<String> parts = TokenizeIgnore(fromString, ",", "()");
	for (int i = 0; i < parts.Size(); ++i)
	{
		String part = parts[i];
		List<String> broken = part.Tokenize("()");
		if (broken.Size() == 0)
		{
			LogShip("Empty movement pattern when parsing ship \'"+name+"\'.");
			continue;
		}
		String partPreParenthesis = broken[0];
		partPreParenthesis.RemoveSurroundingWhitespaces();
		Movement move;
		move.type = -1;
		for (int j = 0; j < Movement::TYPES; ++j)
		{
			String name = Movement::Name(j);
			if (name == partPreParenthesis)
			{
				move.type = j;
				break;
			}
			if (name == "n/a")
				move.type = Movement::NONE;
		}
		if (move.type == -1)
		{
			String typesString;
			for (int j = 0; j < Movement::TYPES; ++j)
			{
				typesString += "\n\t"+Movement::Name(j);
			}
			LogShip("Unrecognized movement pattern \'"+partPreParenthesis+"\' when parsing ship \'"+name+"\'. Available types are as follows: "+typesString);
			continue;
		}
		// Add it and continue.
		if (move.type == Movement::NONE)
		{
			movements.Add(move);
			continue;
		}
		// Demand arguments depending on type?
		if (broken.Size() < 2)
		{
			LogShip("Lacking arguments for movement pattern \'"+partPreParenthesis+"\' when parsing data for ship \'"+name+"\'.");
			continue;
		}
		List<String> args = broken[1].Tokenize(",");
#define DEMAND_ARGS(a) if (args.Size() < a){ \
	LogShip("Expected "+String(a)+" arguments for movement type \'"+\
	Movement::Name(move.type)+"\', encountered "+String(args.Size())+"."); \
	continue;}
#define GET_DURATION(a) if (args[a] == "inf") move.durationMs = -1; else move.durationMs = (int) (args[a].ParseFloat() * 1000);
		switch(move.type)
		{
			case Movement::STRAIGHT:
				DEMAND_ARGS(1);
				GET_DURATION(0);
				break;	
			case Movement::ZAG:
				DEMAND_ARGS(3);
				move.vec.ParseFrom(args[0]);
				move.zagTimeMs = (int) (args[1].ParseFloat() * 1000);
				GET_DURATION(2);
				break;
			case Movement::MOVE_TO:
			{
				DEMAND_ARGS(2);
				// Optionally parse some string for where to go.
				String arg = args[0];
				arg.RemoveSurroundingWhitespaces();
				arg.SetComparisonMode(String::NOT_CASE_SENSITIVE);
				if (arg == "upper edge")
				{
					move.location = Location::UPPER_EDGE; 	
				}
				else if (arg == "lower edge")
				{
					move.location = Location::LOWER_EDGE;
				}
				else if (arg == "center")
				{
					move.location = Location::CENTER;
				}
				else if (arg == "player")
				{
					move.location = Location::PLAYER;
				}
				else 
				{
					move.vec.ParseFrom(args[0]);
					move.location = Location::VECTOR;
				}
				GET_DURATION(1);
				break;
			}
			case Movement::MOVE_DIR:
			{
				DEMAND_ARGS(2);
				move.vec.ParseFrom(args[0]);
				GET_DURATION(1);
				break;
			}
			case Movement::CIRCLE:
			{
				DEMAND_ARGS(4);
				move.target = args[0];
				move.radius = args[1].ParseFloat();
				move.clockwise = args[2].ParseBool();
				GET_DURATION(3);
				break;
			}
			case Movement::UP_N_DOWN:
			{
				DEMAND_ARGS(2);
				move.distance = args[0].ParseFloat();
				GET_DURATION(1);
				break;
			}
		}
		move.vec.Normalize();
		movements.Add(move);
	}
	if (movements.Size() == 0)
	{
		movements.Add(Movement(Movement::STRAIGHT));
	}
	assert(movements.Size());
}


/// E.g. "DoveDir(3), RotateToFace(player, 5)"
void Ship::ParseRotation(String fromString)
{
	rotations.Clear();
	/// Tokenize.
	List<String> parts = TokenizeIgnore(fromString, ",", "()");
	for (int i = 0; i < parts.Size(); ++i)
	{
		String part = parts[i];
		List<String> broken = part.Tokenize("()");
		if (broken.Size() == 0)
		{
			LogShip("Empty movement pattern when parsing ship \'"+name+"\'.");
			continue;
		}
		String partPreParenthesis = broken[0];
		partPreParenthesis.RemoveSurroundingWhitespaces();
		Rotation rota;
		rota.type = -1;
		String name = partPreParenthesis;
		if (name == "MoveDir")
			rota.type = Rotation::MOVE_DIR;
		else if (name == "RotateTo" || name == "RotationTo" || name == "RotateToFace")
			rota.type = Rotation::ROTATE_TO_FACE;
		else if (name == "WeaponTarget")
			rota.type = Rotation::WEAPON_TARGET;
		else if (name == "None" || name == "n/a")
			rota.type = Rotation::NONE;
		if (rota.type == -1)
		{
			LogShip("Unrecognized movement pattern \'"+name+"\' when parsing ship \'"+name+"\'. Available types are as follows: \n\
																						   MoveDir(duration)\
																						   RotateToFace(location, duration)");
			continue;
		}
		// Add it and continue.
		if (rota.type == Rotation::NONE)
		{
			rotations.Add(rota);
			continue;
		}
		// Demand arguments depending on type?
		if (broken.Size() < 2)
		{
			LogShip("Lacking arguments for movement pattern \'"+partPreParenthesis+"\' when parsing data for ship \'"+name+"\'.");
			continue;
		}
		List<String> args = broken[1].Tokenize(",");
#undef DEMAND_ARGS
#define DEMAND_ARGS(a) if (args.Size() < a){ \
	LogShip("Expected "+String(a)+" arguments for movement type \'"+\
	Rotation::Name(rota.type)+"\', encountered "+String(args.Size())+"."); \
	continue;}
#undef GET_DURATION
#define GET_DURATION(a) if (args[a] == "inf") rota.durationMs = -1; else rota.durationMs = (int)(args[a].ParseFloat() * 1000);
		switch(rota.type)
		{
			case Rotation::NONE:
				break;
			case Rotation::MOVE_DIR:
			case Rotation::WEAPON_TARGET: 
				DEMAND_ARGS(1);				// Just durations here.
				GET_DURATION(0);
				break;	
			case Rotation::ROTATE_TO_FACE:
				DEMAND_ARGS(2);
				rota.target = args[0];
				GET_DURATION(1);
				break;
			case Rotation::SPINNING:
				DEMAND_ARGS(2);
				rota.spinSpeed = args[0].ParseFloat();
				GET_DURATION(1);
				break;
		}
		rotations.Add(rota);
	}
	if (rotations.Size() == 0)
	{
		rotations.Add(Rotation(Rotation::NONE));
	}
	assert(rotations.Size());
}

