/// Emil Hedemalm
/// 2015-01-21
/// Ship.

#include "../SpaceShooter2D.h"
#include "Ship.h"

#include "File/File.h"
#include "String/StringUtil.h"

#include "File/LogFile.h"
#include "Game/GameVariable.h"
#include "SpaceShooter2D/Level/SpawnGroup.h"
#include "WeaponScript.h"
#include "SpaceShooter2D/SpaceShooterScript.h"

int Ship::shipIDEnumerator = 0;

extern SpaceShooterEvaluator spaceShooterEvaluator;

Ship::Ship()
{
	shipID = ++shipIDEnumerator;
	despawnOutsideFrame = true;
	script = 0;
	collisionDamageCooldown = Time(TimeType::MILLISECONDS_NO_CALENDER, 100);
	lastShipCollision = Time(TimeType::MILLISECONDS_NO_CALENDER, 0);

	heatDamageTaken = 0;
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

Random cooldownRand;
void Ship::RandomizeWeaponCooldowns()
{
//	if (!ai)
//		return;
	for (int i = 0; i < weapons.Size(); ++i)
	{
		Weapon * weap = weapons[i];
		weap->currCooldownMs = weap->cooldown.Milliseconds() * cooldownRand.Randf();
//		weap->lastShot = /*flyTime +*/ Time::Milliseconds(weap->cooldown.Milliseconds() * cooldownRand.Randf());
	}
}

List<Entity*> Ship::Spawn(ConstVec3fr atPosition, Ship * in_parent)
{
	name.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (name.Contains("boss"))
		despawnOutsideFrame = false;
	parent = in_parent;
	if (parent)
	{
		parent->children.AddItem(this);
	}

	Entity * entity = EntityMan.CreateEntity(name, GetModel(), TexMan.GetTextureByColor(Color(0,255,0,255)));
	entity->localPosition = atPosition;
	float radians = PI / 2;
//		entity->rotation[0] = radians; // Rotate up from Z- to Y+
//		entity->rotation[1] = radians; // Rorate from Y+ to X-
	/// No default rotation.
	if (!parent)
		entity->SetRotation(Vector3f(radians, radians, 0));
	
	PhysicsProperty * pp = new PhysicsProperty();
	entity->physics = pp;
	// Setup physics.
	pp->type = PhysicsType::DYNAMIC;
	pp->collisionCategory = CC_ENEMY;
	pp->collisionFilter = CC_PLAYER | CC_PLAYER_PROJ;
	pp->collisionCallback = true;
	pp->shapeType = PhysicsShape::AABB;
	// By default, set invulerability on spawn.
	this->spawnInvulnerability = true;
	ShipProperty * sp = new ShipProperty(this, entity);
	entity->properties.Add(sp);
	this->entity = entity;
	this->spawned = true;
	this->StartMovement();

	/// Spawn children if applicable.
	List<Entity*> children = SpawnChildren();
	/// Set up parenting.
	if (parent)
	{
		parent->entity->children.AddItem(this->entity);
		this->entity->parent = parent->entity;
//		QueuePhysics(new PMSetEntity(entity, PT_PARENT, parent->entity));
	}
	/// IF final aprent, register for rendering, etc.
	else 
	{
		List<Entity*> all = children + entity;
		shipEntities.Add(all);
		MapMan.AddEntities(all);
		/// Recalculate matrix and all children matrices.
		entity->RecalculateMatrix(Entity::ALL_PARTS, true);
	}
	/// Load script if applicable.
	if (scriptSource.Length())
	{
		script = new SpaceShooterScript();
		script->Load(scriptSource);
		script->entity = entity;
		/// Add custom variables based on who started the script.
		script->variables.AddItem(Variable("self", shipID));
		script->variables.AddItem(Variable("player", playerShip->shipID));
		script->functionEvaluators.AddItem(&spaceShooterEvaluator);
		script->OnBegin();
	}

	return entity;
}

/// Handles spawning of children as needed.
List<Entity*> Ship::SpawnChildren()
{
	/// Translate strings.
	List<String> childStrings;
	for (int i = 0; i < childrenStrings.Size(); ++i)
	{
		String str = childrenStrings[i];
		if (str.EndsWith('*'))
		{
			String strWoStar = str - "*";
			/// Grab all starting with it.
			for (int j = 0; j < Ship::types.Size(); ++j)
			{
				Ship * type = Ship::types[j];
				if (type->name.Contains(strWoStar))
					childStrings.AddItem(type->name);
			}
		}
	}
	List<Entity*> childrenSpawned;
	for (int i = 0; i < childStrings.Size(); ++i)
	{
		String str = childStrings[i];
		Ship * newShip = Ship::New(str);
		if (!newShip)
		{
			LogMain("Ship::SpawnChildren: Unable to create ship of type: "+str, ERROR | CAUSE_ASSERTION_ERROR);
			continue;
		}
		activeLevel->ships.AddItem(newShip);
		Ship * ship = newShip;
		ship->allied = this->allied;
		ship->RandomizeWeaponCooldowns();
		ship->spawnGroup = this->spawnGroup;
		ship->Spawn(Vector3f(), this);
		childrenSpawned.AddItem(ship->entity);
		/// Apply spawn group properties.
//		ship->shoot &= shoot;
//		ship->speed *= relativeSpeed;
	}
	return childrenSpawned;
}

void Ship::Despawn()
{
	LogMain("Despawning ship "+name, INFO);
	/// Notify parent if child.
	if (parent)
	{
		parent->children.RemoveItem(this);
		parent = 0;
	}
	for (int i = 0; i < children.Size(); ++i)
	{
		Ship * child = children[i];
		child->Despawn();
	}
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

/// Checks current movement. Will only return true if movement is target based and destination is within threshold.
bool Ship::ArrivedAtDestination()
{
//	LogMain("Update maybe", INFO);
	return false;
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
	/// Process scripts (pretty much AI and other stuff?)
	if (script)
		script->Process(timeInMs);
	// Increment time in movement if applicable.
	if (movementPatterns.Size())
		movementPatterns[currentMovement].OnFrame(timeInMs);
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
			spaceShooter->UpdateUIPlayerShield(false);
	}
	if (allied)
	{
		hp += timeInMs * armorRegenRate * 0.001f;
		if (hp > maxHP)
			hp = maxHP;
		spaceShooter->UpdateUIPlayerHP(false);
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

	/// Process ze weapons.
	for (int i = 0; i < weapons.Size(); ++i)
		weapons[i]->Process(this, timeInMs);

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

/// Disables weapon in this and children ships.
void Ship::DisableWeapon(String weaponName)
{
	for (int i = 0; i < weapons.Size(); ++i)
	{
		Weapon * weap = weapons[i];
		if (weap->name == weaponName)
			weap->enabled = false;
	}
	for (int i = 0; i < children.Size(); ++i)
	{
		children[i]->DisableWeapon(weaponName);
	}
}


/// Prepends the source with '/obj/Ships/' and appends '.obj'. Uses default 'Ship.obj' if needed.
Model * Ship::GetModel()
{
	String folder = "obj/";//Ships/";
	Model * model = ModelMan.GetModel(graphicModel);
	if (!model)
	{
		std::cout<<"\nUnable to find ship model with name \'"<<graphicModel<<"\', using default model.";
		graphicModel.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (graphicModel.Contains("turret"))
			model = ModelMan.GetModel("obj/Ships/Turret");
		else
			model = ModelMan.GetModel("obj/Ships/Ship");
	}
	return model;
}

void Ship::DisableMovement()
{
	movementDisabled = true;
}
void Ship::OnSpeedUpdated()
{
	/// Update based on current movement?
	if (movementPatterns.Size())
		movementPatterns[currentMovement].OnSpeedUpdated();
}


void Ship::Damage(Weapon & weapon)
{
	float damage = weapon.damage * weapon.relativeStrength;
	bool ignoreShield = false;
	if (weapon.type == HEAT_WAVE)
	{
		heatDamageTaken += damage;
		damage += heatDamageTaken;
	}
	Damage(damage, ignoreShield);
}

extern bool playerInvulnerability;

void Ship::Damage(float amount, bool ignoreShield)
{
	if (allied && playerInvulnerability)
		return;
	if (!ai)
		LogMain("Player took "+String((int)amount)+" damage!", INFO);
	if (spawnInvulnerability)
	{
	//	std::cout<<"\nInvulnnnn!";
	//	return;
	}
	if (hasShield && !ignoreShield)
	{
		shieldValue -= amount;
		amount = -shieldValue;
		if (shieldValue < 0 )
			shieldValue = 0;
		if (this->allied)
			spaceShooter->UpdateUIPlayerShield(true);
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
	amount = (amount / (activeToughness / 10.f));

	hp -= amount;
	if (hp < 0)
		hp = 0;
	if (this->allied)
		spaceShooter->UpdateUIPlayerHP(false);
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
		SparksEmitter * tmpEmitter = new SparksEmitter(entity->worldPosition);
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
		/// SFX
		QueueAudio(new AMPlaySFX("sfx/Ship Death.wav"));
	}
}


void Ship::SetMovement(Movement & movement)
{
	this->movementPatterns.Clear();
//		move.vec = Vector2f(-10.f, targetEntity->worldPosition.y); 
	this->movementPatterns.AddItem(movement);
	movementPatterns[0].OnEnter(this);
//	.OnEnter(ship);
}

void Ship::SetSpeed(float newSpeed)
{
	this->speed = newSpeed;
	this->OnSpeedUpdated();
}

/// Creates new ship of specified type.
Ship * Ship::New(String shipByName)
{
//	shipByName.Replace('_', ' '); // Move this elsewhere?
	shipByName.RemoveSurroundingWhitespaces(); // Move this elsewhere?
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
				*newWeap = *Weapon::Get(refWeap->type, refWeap->level);
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
//	UpdateStatsFromGear();
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
	Weapon * targetWeapon = Weapon::Get(weaponType, level);
	if (!targetWeapon)
	{
		if (level == 0)
		{
			targetWeapon = Weapon::Get(weaponType, 1);
			if (targetWeapon)
			{
				*weapon = *targetWeapon;
				weapon->level = 0;
				weapon->name = "";
				weapon->cooldown = Time(TimeType::MILLISECONDS_NO_CALENDER, 4000000);
			}
		}
		return;
	}
	*weapon = *targetWeapon;
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
