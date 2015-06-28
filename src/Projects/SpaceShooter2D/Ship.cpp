/// Emil Hedemalm
/// 2015-01-21
/// Ship.

#include "SpaceShooter2D.h"
#include "Ship.h"

#include "File/File.h"
#include "String/StringUtil.h"

#include "File/LogFile.h"
#include "Game/GameVariable.h"
#include "Level/SpawnGroup.h"

ScriptAction ScriptAction::SwitchWeapon(int toWeaponIndex, int durationToHoldMs)
{
	ScriptAction sa;
	sa.type = SWITCH_TO_WEAPON;
	sa.weaponIndex = toWeaponIndex;
	sa.durationMs = durationToHoldMs;
	return sa;
}

void ScriptAction::OnEnter(Ship * forShip)
{
	if (type == SWITCH_TO_WEAPON)
	{
		forShip->activeWeapon = forShip->weapons[weaponIndex];
		forShip->shoot = true;
	}
}

WeaponScript::WeaponScript()
{
	timeInCurrentActionMs = 0;
	currentAction = 0;
}

void WeaponScript::Process(Ship * forShip, int timeInMs)
{
	assert(actions.Size());
	timeInCurrentActionMs += timeInMs;
	ScriptAction & current = actions[currentAction];
	if (timeInCurrentActionMs > current.durationMs)
	{
		currentAction = (currentAction + 1) % actions.Size();
		// When entering a new one, do stuff.
		ScriptAction & newOne = actions[currentAction];
		newOne.OnEnter(forShip);
		timeInCurrentActionMs = 0;
	}
}

Ship::Ship()
{
	collisionDamageCooldown = Time(TimeType::MILLISECONDS_NO_CALENDER, 100);
	lastShipCollision = Time(TimeType::MILLISECONDS_NO_CALENDER, 0);

	spawnGroup = 0;
	shoot = false;
	activeWeapon = 0;
	spawned = false;
	entity = NULL;
	ai = true;
	allied = false;
	maxHP = hp = 10;
	canMove = false;
	canShoot = true;
	spawnInvulnerability = false;
	hasShield = false;
	shieldValue = maxShieldValue = 10;
	currentMovement = 0;
	timeInCurrentMovement = 0;
	speed = 0.f;
	score = 10;
	destroyed = false;
	spawned = false;

	currentRotation = 0;
	timeInCurrentRotation = 0;

	collideDamage = 1;
	armorRegenRate = 2;
	
	graphicModel = "obj/Ships/Ship.obj";

	timeSinceLastSkillUseMs = 0;
	maxRadiansPerSecond = PI / 12;
	movementDisabled = false;
	weaponScriptActive = false;
	weaponScript = 0;
	activeSkill = NO_SKILL;
	skill = NO_SKILL;
	skillCooldownMultiplier = 1.f;
}

Ship::~Ship()
{
}

void Ship::Despawn()
{
	if (spawnGroup)
	{
		if (hp <= 0)
			spawnGroup->OnShipDestroyed(this);
		else
			spawnGroup->OnShipDespawned(this);
	}
	MapMan.DeleteEntity(entity);
	shipEntities.Remove(entity);
	entity = NULL;
}

void Ship::Process(int timeInMs)
{
	// Skill cooldown.
	if (timeSinceLastSkillUseMs >= 0)
	{
		timeSinceLastSkillUseMs += timeInMs;
		if (timeSinceLastSkillUseMs > skillDurationMs)
		{
			activeSkill = NO_SKILL;
			spaceShooter->UpdateHUDSkill();
			if (timeSinceLastSkillUseMs > skillCooldownMs)
				timeSinceLastSkillUseMs = -1; // Ready to use.
		}
	}
	// AI
	ProcessAI(timeInMs);
	// Weapon systems.
	ProcessWeapons(timeInMs);
	// Shield
	if (hasShield)
	{
		// Repair shield
		shieldValue += timeInMs * shieldRegenRate * (activeSkill == POWER_SHIELD? 10.f : 0.001);
		if (shieldValue > MaxShield())
			shieldValue = MaxShield();
		if (allied)
			spaceShooter->UpdateUIPlayerShield();
	}
	if (allied)
	{
		hp += timeInMs * armorRegenRate * 0.001f;
		if (hp > maxHP)
			hp = maxHP;
		spaceShooter->UpdateUIPlayerHP();
	}
}

void Ship::ProcessAI(int timeInMs)
{
	// Don't process inactive ships..
	if (!ai)
		return;
	if (rotationPatterns.Size() == 0)
		return;
	// Rotate accordingly.
	Rotation & rota = rotationPatterns[currentRotation];
	rota.OnFrame(timeInMs);
	// Increase time spent in this state accordingly.
	timeInCurrentRotation += timeInMs;
	if (timeInCurrentRotation > rota.durationMs && rota.durationMs > 0)
	{
		currentRotation = (currentRotation + 1) % rotationPatterns.Size();
		timeInCurrentRotation = 0;
		Rotation & rota2 = rotationPatterns[currentRotation];
		rota2.OnEnter(this);
	}
	if (!canMove)
		return;
	// Move?
	Entity * shipEntity = entity;
	Movement & move = movementPatterns[currentMovement];
	move.OnFrame(timeInMs);
	// Increase time spent in this state accordingly.
	timeInCurrentMovement += timeInMs;
	if (timeInCurrentMovement > move.durationMs && move.durationMs > 0)
	{
		currentMovement = (currentMovement + 1) % movementPatterns.Size();
		timeInCurrentMovement = 0;
		Movement & newMove = movementPatterns[currentMovement];
		newMove.OnEnter(this);
	}
}


void Ship::ProcessWeapons(int timeInMs)
{
	if (!canShoot)
		return;

	if (weaponScriptActive && weaponScript)
	{
		weaponScript->Process(this, timeInMs);
	}

	// AI fire all weapons simultaneously for the time being.
	if (ai)
	{
		// Do stuff.
		for (int i = 0; i < weapons.Size(); ++i)
		{
			Weapon * weapon = weapons[i];
			// Aim.
			weapon->Aim(this);
			// Dude..
			if (projectileEntities.Size() > 1000)
				continue;
			weapon->Shoot(this);
		}
	}
	else 
	{
		if (!shoot)
			return;
		if (activeWeapon == 0)
			activeWeapon = weapons.Size()? weapons[0] : 0;
		// Shoot with current weapon for player.
		if (activeWeapon)
			activeWeapon->Shoot(this);
	}
}

/// Prepends the source with '/obj/Ships/' and appends '.obj'. Uses default 'Ship.obj' if needed.
Model * Ship::GetModel()
{
	String folder = "obj/Ships/";
	Model * model = ModelMan.GetModel(folder + graphicModel);
	if (!model)
	{
		std::cout<<"\nUnable to find ship model with name \'"<<graphicModel<<"\', using default model.";
		graphicModel.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (graphicModel.Contains("turret"))
			model = ModelMan.GetModel(folder + "Turret");
		else
			model = ModelMan.GetModel(folder + "Ship");
	}
	return model;
}

void Ship::DisableMovement()
{
	movementDisabled = true;
}

void Ship::Damage(int amount, bool ignoreShield)
{
	if (spawnInvulnerability)
	{
	//	std::cout<<"\nInvulnnnn!";
	//	return;
	}
	if (hasShield && !ignoreShield)
	{
		shieldValue -= amount;
		amount = (int) -shieldValue;
		if (shieldValue < 0 )
			shieldValue = 0;
		if (this->allied)
			spaceShooter->UpdateUIPlayerShield();
		if (amount < 0)
			return;
		shieldValue = 0;
	}
	// Modulate amount depending on armor toughness and reactivity.
	float activeToughness = (float)armor.toughness;
	/// Projectile/explosion-type attacks, reactivity effects.
	if (!ignoreShield)
	{
		activeToughness += armor.reactivity;
	}
	amount = (int)(amount / (activeToughness / 10.f));

	hp -= amount;
	if (this->allied)
		spaceShooter->UpdateUIPlayerHP();
	if (hp <= 0)
		Destroy();
}

void Ship::Destroy()
{
	if (entity)
	{
		if (spawnGroup)
		{
			spawnGroup->OnShipDestroyed(this);
		}
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
		shipEntities.Remove(entity);
		entity = NULL;
		destroyed = true;
		// Explosion?
		// Increase score and kills.
		if (!allied)
		{
			spaceShooter->LevelScore()->iValue += this->score;
			spaceShooter->LevelKills()->iValue++;
		}
		else 
			failedToSurvive = true;
	}
}


String whenParsingFile;
int row = 0;
#define LogShip(s) LogToFile("ShipParserErrors.txt", whenParsingFile + " row "+String(row+1) + String(": ") + s)
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
	int commas = firstLine.Count(',');
	int semiColons = firstLine.Count(';');
	int delimiter = semiColons > commas? ';' : ',';
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
			else if (column == "OnCollision")
				ship->onCollision = value;
			else if (column == "Can Move")
				ship->canMove = value.ParseBool();
			else if (column == "Can Shoot")
				ship->canShoot = value.ParseBool();
			else if (column == "Weapon Name")
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
						LogShip("Unable to find weapon by name \'"+name+"\' when parsing ship \'"+ship->name+"\'");
						delete weapon;
					}	
				}
			}
			else if (column == "Weapon Ammunition")
			{
				List<String> ammos = value.Tokenize(",");
				if (ammos.Size() != ship->weapons.Size())
				{
					LogShip("ERROR: Trying to assign "+String(ammos.Size())+" ammunitions to "+String(ship->weapons.Size())+" weapons when parsing ship \'"+ship->name+"\'");
					continue;
				}
				for (int i = 0; i < ammos.Size(); ++i)
				{
					Weapon * weapon = ship->weapons[i];
					String ammo = ammos[i];
					if (ammo == "inf")
						weapon->ammunition = -1;
					weapon->ammunition = ammo.ParseInt();
				}
			}
			else if (column == "Weapon Cooldown")
			{
				if (ship->weapons.Size() == 0)
				{
					LogShip("ERROR: when trying to assign weapon cooldown, no valid weapon.");
					continue;
				}
				if (value.Contains("Burst"))
				{
					// PArse burst> Burst(rounds, delayBetweenShots, cooldown)
					Weapon * weapon = ship->weapons[0];
					List<String> toks = value.Tokenize("(),");
					weapon->burst = true;
					weapon->burstRounds = toks[1].ParseInt();
					weapon->burstRoundDelay.intervals = (int) (toks[2].ParseFloat() * 1000);
					weapon->cooldown.intervals = (int) (toks[3].ParseFloat() * 1000);
				}
				else 
				{
					List<String> cooldowns = value.Tokenize(",");
					if (cooldowns.Size() != ship->weapons.Size())
					{
						// Print error message
						LogShip("Trying to assign "+String(cooldowns.Size())+" weapon cooldowns to "+String(ship->weapons.Size())+" weapons when parsing ship \'"+ship->name+"\'");
						continue;
					}
					for (int i = 0; i < ship->weapons.Size(); ++i)
					{
						Weapon * weapon = ship->weapons[i];
						weapon->cooldown.intervals = (int) (cooldowns[i].ParseFloat() * 1000);
						int p = i;
					}
				}
			}
			else if (column == "Movement pattern")
			{
				ship->ParseMovement(value);
				ship->movementPatterns.Size();
			}
			else if (column == "Rotation pattern")
			{
				ship->ParseRotation(value);
				ship->rotationPatterns.Size();
			}
			else if (column == "Score")
				ship->score = value.ParseInt();
			else if (column == "Speed")
				ship->speed = value.ParseFloat() * 0.2f;
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
			else if (column == "Max rotation per second" || column == "Rotation Speed")
				ship->maxRadiansPerSecond = DEGREES_TO_RADIANS(value.ParseInt());
			else if (column == "Graphic model")
				ship->graphicModel = value;
			else 
			{
				LogShip("Unrecognized column name \'"+column+"\'.");
			}
		}
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
		movementPatterns.Add(move);
	}
	if (movementPatterns.Size() == 0)
	{
		movementPatterns.Add(Movement(Movement::NONE));
	}
	assert(movementPatterns.Size());
}


/// E.g. "DoveDir(3), RotateToFace(player, 5)"
void Ship::ParseRotation(String fromString)
{
	rotationPatterns.Clear();
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
			rotationPatterns.Add(rota);
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
		rotationPatterns.Add(rota);
	}
	if (rotationPatterns.Size() == 0)
	{
		rotationPatterns.Add(Rotation(Rotation::NONE));
	}
	assert(rotationPatterns.Size());
}

/// Creates new ship of specified type.
Ship * Ship::New(String shipByName)
{
	shipByName.Replace('_', ' ');
	shipByName.RemoveSurroundingWhitespaces();
	List<String> typesNames;
	for (int i = 0; i < types.Size(); ++i)
	{
		Ship * type = types[i];
		if (type->name == shipByName)
		{
			Ship * newShip = new Ship(*type);
			// Create copies of the weapons.
			newShip->weapons.Clear();
			
			return newShip;
		}
		typesNames.Add(type->name);
	}
	String shipTypesStr = MergeLines(typesNames, ", ");
	// For now, just add a default one.
	LogToFile("LevelCreationErrors.txt", "ERROR: Couldn't find ship by name \'"+shipByName+"\'. Available ships types as follows:\n\t" + shipTypesStr, ERROR);
	std::cout<<"\nERROR: Couldn't find ship by name \'"<<shipByName<<"\'";
	return 0;
}

/// Returns speed, accounting for active skills, weights, etc.
float Ship::Speed()
{
	if (activeSkill == SPEED_BOOST)
		return speed * 1.75f;
	return speed;
}

/// Accounting for boosting skills.
float Ship::MaxShield()
{
	if (activeSkill == POWER_SHIELD)
		return maxShieldValue + 10000;
	return maxShieldValue;
}

/// Checks weapon's latest aim dir.
Vector3f Ship::WeaponTargetDir()
{
	for (int i = 0; i < weapons.Size(); ++i)
	{
		Weapon * weapon = weapons[i];
		if (weapon->aim)
			return weapon->currentAim;
	}
	return Vector3f();
}

/// If using Armor and Shield gear (Player mainly).
void Ship::UpdateStatsFromGear()
{
	hp = this->maxHP = armor.maxHP;
	shieldValue = (float) (this->maxShieldValue = shield.maxShield);
	this->shieldRegenRate = shield.shieldRegen;
}

bool Ship::SwitchToWeapon(int index)
{
	if (index < 0 || index >= weapons.Size())
	{
		std::cout<<"\nSwitchToWeapon bad index";
		return false;
	}
	activeWeapon = weapons[index];
	std::cout<<"\nSwitched to weapon: "<<activeWeapon->name;
	UpdateStatsFromGear();
	// Update ui
	if (!ai)
	{
		QueueGraphics(new GMSetUIi("ActiveWeapon", GMUI::INTEGER_INPUT, index));
	}
	return true;
}

/// Calls OnEnter for the initial movement pattern.
void Ship::StartMovement()
{
	if (rotationPatterns.Size())
		rotationPatterns[0].OnEnter(this);
	if (movementPatterns.Size())
		movementPatterns[0].OnEnter(this);
}

/// For player ship.
void Ship::SetWeaponLevel(int weaponType, int level)
{
	Weapon * weapon = GetWeapon(weaponType);
	*weapon = Weapon::Get(weaponType, level);
}

Weapon * Ship::GetWeapon(int ofType)
{
	for (int i = 0; i < weapons.Size(); ++i)
	{
		Weapon * weapon = weapons[i];
		if (weapon->type == ofType)
			return weapon;
	}
	Weapon * newWeapon = new Weapon();
	newWeapon->type = ofType;
	newWeapon->level = 0;
	weapons.AddItem(newWeapon);
	return weapons.Last();
}

void Ship::ActivateSkill()
{
	// Check cooldown?
	if (timeSinceLastSkillUseMs != -1)
		return;
	activeSkill = skill;
	timeSinceLastSkillUseMs = 0;
	skillDurationMs = 5000;
	switch(skill)
	{
		case POWER_SHIELD:	
			skillCooldownMs = 25000; 
			break;
		case SPEED_BOOST: 	
			skillCooldownMs = 20000;
			break;
		case ATTACK_FRENZY: 
			skillCooldownMs = 30000; 
			break;
		default:
			skillCooldownMs = skillDurationMs = 100;
	}
	skillCooldownMs *= skillCooldownMultiplier;
	// Reflect activation in HUD?
	spaceShooter->UpdateHUDSkill();
}
