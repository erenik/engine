/// Emil Hedemalm
/// 2015-01-21
/// Ship.

#include "SpaceShooter2D.h"
#include "Ship.h"

#include "File/File.h"
#include "String/StringUtil.h"

#include "File/LogFile.h"

Ship::Ship()
{
	spawned = false;
	entity = NULL;
	ai = true;
	allied = false;
	maxHitPoints = hitPoints = 10;
	canMove = false;
	canShoot = true;
	spawnInvulnerability = false;
	hasShield = false;
	shieldValue = maxShieldValue = 10;
	currentMovement = 0;
	timeInCurrentMovement = 0;
	speed = 0.f;
}

Ship::~Ship()
{
	
}

void Ship::Damage(int amount, bool ignoreShield)
{
	if (spawnInvulnerability)
	{
	//	std::cout<<"\nInvulnnnn!";
		return;
	}			
	if (hasShield && !ignoreShield)
	{
		shieldValue -= amount;
		amount = -shieldValue;
		if (shieldValue < 0 )
			shieldValue = 0;
		if (this->allied)
			spaceShooter->UpdatePlayerShield();
		if (amount < 0)
			return;
		shieldValue = 0;
	}
	hitPoints -= amount;
	if (this->allied)
		spaceShooter->UpdatePlayerHP();
	if (hitPoints <= 0)
		Destroy();
}

void Ship::Destroy()
{
	if (entity)
	{
		ShipProperty * sp = entity->GetProperty<ShipProperty>();
		if (sp)
			sp->sleeping = true;
		// Add a temporary emitter to the particle system to add some sparks to the collision
		SparksEmitter * tmpEmitter = new SparksEmitter(entity->position);
		tmpEmitter->SetRatioRandomVelocity(1.0f);
		tmpEmitter->SetEmissionVelocity(4.5f);
		tmpEmitter->constantEmission = 750;
		tmpEmitter->instantaneous = true;
		tmpEmitter->SetScale(0.2f);
		tmpEmitter->SetParticleLifeTime(3.5f);
		tmpEmitter->SetColor(Vector4f(1.f, 0.5f, 0.1f, 1.f));
		Graphics.QueueMessage(new GMAttachParticleEmitter(tmpEmitter, sparks));

		MapMan.DeleteEntity(entity);
		spaceShooter->shipEntities.Remove(entity);
		entity = NULL;
		// Explosion?
	}
}


String whenParsingFile;
int row = 0;
#define LogShip(s) LogToFile("ShipParserErrors.txt", whenParsingFile + " row "+String(row) + String(": ") + s)
// Load ship-types.
bool Ship::LoadTypes(String file)
{
	whenParsingFile = file;
	List<String> lines = File::GetLines(file);
	if (lines.Size() == 0)
		return false;

	String separator;
	/// Column-names. Parse from the first line.
	List<String> columns;
	String firstLine = lines[0];
	// Keep empty strings or all will break.
	columns = TokenizeCSV(firstLine);

	// For each line after the first one, parse data.
	for (int j = 1; j < lines.Size(); ++j)
	{
		String & line = lines[j];
		row = j;
		// Keep empty strings or all will break.
		List<String> values = TokenizeCSV(line);
		for (int i = 0; i < values.Size(); ++i)
		{
			String v = values[i];
	//		std::cout<<"\nValue "<<i<<": "<<v;
		}
		// If not, now loop through the words, parsing them according to the column name.
		// First create the new spell to load the data into!
		Ship ship;
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
				ship.name = value;
			else if (column == "Type")
				ship.type = value;
			else if (column == "Can Move")
				ship.canMove = value.ParseBool();
			else if (column == "Can Shoot")
				ship.canShoot = value.ParseBool();
			else if (column == "Weapon Name")
			{
				List<String> weaponNames = value.Tokenize(",");
				// Fetch weapons?
				for (int i = 0; i < weaponNames.Size(); ++i)
				{
					String name = weaponNames[i];
					name.RemoveInitialWhitespaces();
					name.RemoveTrailingWhitespaces();
					Weapon weapon; 
					bool ok = Weapon::Get(name, weapon);
					if (ok)
						ship.weapons.Add(weapon);
					else 
						LogShip("Unable to find weapon by name \'"+name+"\' when parsing ship \'"+ship.name+"\'");
				}
			}
			else if (column == "Weapon Ammunition")
			{
				List<String> ammos = value.Tokenize(",");
				if (ammos.Size() != ship.weapons.Size())
				{
					LogShip("ERROR: Trying to assign "+String(ammos.Size())+" ammunitions to "+String(ship.weapons.Size())+" weapons when parsing ship \'"+ship.name+"\'");
					continue;
				}
				for (int i = 0; i < ammos.Size(); ++i)
				{
					Weapon & weapon = ship.weapons[i];
					String ammo = ammos[i];
					if (ammo == "inf")
						weapon.ammunition = -1;
					weapon.ammunition = ammo.ParseInt();
				}
			}
			else if (column == "Weapon Cooldown")
			{
				if (ship.weapons.Size() == 0)
				{
					LogShip("ERROR: when trying to assign weapon cooldown, no valid weapon.");
					continue;
				}
				if (value.Contains("Burst"))
				{
					// PArse burst> Burst(rounds, delayBetweenShots, cooldown)
					Weapon & weapon = ship.weapons[0];
					List<String> toks = value.Tokenize("(),");
					weapon.burst = true;
					weapon.burstRounds = toks[1].ParseInt();
					weapon.burstRoundDelayMs = (int) (toks[2].ParseFloat() * 1000);
					weapon.cooldownMs = (int) (toks[3].ParseFloat() * 1000);
				}
				else 
				{
					List<String> cooldowns = value.Tokenize(",");
					if (cooldowns.Size() != ship.weapons.Size())
					{
						// Print error message
						LogShip("Trying to assign "+String(cooldowns.Size())+" weapon cooldowns to "+String(ship.weapons.Size())+" weapons when parsing ship \'"+ship.name+"\'");
						continue;
					}
					for (int i = 0; i < ship.weapons.Size(); ++i)
					{
						Weapon & weapon = ship.weapons[i];
						weapon.cooldownMs = (int) (cooldowns[i].ParseFloat() * 1000);
						int p = i;
					}
				}
			}
			else if (column == "Movement pattern")
			{
				ship.ParseMovement(value);
				ship.movementPatterns.Size();
			}
			else if (column == "Speed")
				ship.speed = value.ParseFloat() * 0.2f;
			else if (column == "Has Shield")
				ship.hasShield = value.ParseBool();
			else if (column == "Shield Value")
				ship.shieldValue = ship.maxShieldValue = value.ParseInt();
			else if (column == "Shield Regen Rate")
				ship.shieldRegenRate = value.ParseInt() / 1000.f;
			else if (column == "Hit points")
				ship.maxHitPoints = ship.hitPoints = value.ParseInt();
			else if (column == "Graphic model")
				ship.graphicModel = value;
		}
		// Check for pre-existing ship of same name, remove it if so.
		for (int i = 0; i < types.Size(); ++i)
		{
			Ship & type = types[i];
			if (type.name == ship.name)
			{
				types.RemoveIndex(i);
				--i;
			}
		}
		types.Add(ship);
	}
	return true;
}

/// E.g. "Straight(10), MoveTo(X Y 5 20, 5)"
void Ship::ParseMovement(String fromString)
{
	movementPatterns.Clear();
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
			movementPatterns.Add(move);
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
#define GET_DURATION(a) if (args[a] == "inf") move.durationMs = -1; else move.durationMs = args[a].ParseFloat() * 1000;
		switch(move.type)
		{
			case Movement::STRAIGHT:
				DEMAND_ARGS(1);
				GET_DURATION(0);
				break;	
			case Movement::ZAG:
				DEMAND_ARGS(3);
				move.vec.ParseFrom(args[0]);
				move.zagTimeMs = args[1].ParseFloat() * 1000;
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
		movementPatterns.Add(move);
	}
	if (movementPatterns.Size() == 0)
	{
		movementPatterns.Add(Movement(Movement::NONE));
	}
	assert(movementPatterns.Size());
}


/// Creates new ship of specified type.
Ship Ship::New(String shipByName)
{
	shipByName.Replace('_', ' ');
	for (int i = 0; i < types.Size(); ++i)
	{
		Ship & type = types[i];
		if (type.name == shipByName)
			return type;
	}
	// For now, just add a default one.
	std::cout<<"\nERROR: Couldn't find ship by name \'"<<shipByName<<"\'";
	return Ship();
}


/// Calls OnEnter for the initial movement pattern.
void Ship::StartMovement()
{
	if (!canMove)
		return;
	assert(movementPatterns.Size());
	if (movementPatterns.Size())
		movementPatterns[0].OnEnter(this);
}
