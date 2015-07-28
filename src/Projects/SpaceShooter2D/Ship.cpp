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
	canShoot = false;
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
	weapons.ClearAndDelete();
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
	if (!weapons.Size())
		return;

	if (weaponScriptActive && weaponScript)
	{
		weaponScript->Process(this, timeInMs);
	}

	// AI fire all weapons simultaneously for the time being.
	if (ai)
	{
		shoot = false;
		if (weapons.Size() == 0)
		{
			std::cout<<"\nLacking weapons..";
		}
		// Do stuff.
		for (int i = 0; i < weapons.Size(); ++i)
		{
			Weapon * weapon = weapons[i];
			// Aim.
			weapon->Aim(this);
			// Dude..
			shoot = true;
			// Shoot all weapons by default.
			weapon->Shoot(this); 
		}
		return;
	}
	if (!shoot)
		return;
	if (activeWeapon == 0)
		activeWeapon = weapons.Size()? weapons[0] : 0;
	// Shoot with current weapon for player.
	if (activeWeapon)
		activeWeapon->Shoot(this);
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
		tmpEmitter->SetEmissionVelocity(2.5f);
		tmpEmitter->constantEmission = 750;
		tmpEmitter->instantaneous = true;
		tmpEmitter->SetScale(0.1f);
		tmpEmitter->SetParticleLifeTime(2.5f);
		Vector4f color =  Vector4f(1,0.5,0.1,1.f);// entity->diffuseMap->averageColor;
		color.w *= 0.5f;
		tmpEmitter->SetColor(color);
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
			for (int j = 0; j < type->weapons.Size(); ++j)
			{
				Weapon * refWeap = type->weapons[j];
				Weapon * newWeap = new Weapon();
				*newWeap = Weapon::Get(refWeap->type, refWeap->level);
				newShip->weapons.AddItem(newWeap);
			}
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
