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
#include "../Properties/ShipProperty.h"

int Ship::shipIDEnumerator = 0;

extern SpaceShooterEvaluator spaceShooterEvaluator;

Ship::Ship()
{
	shipProperty = 0;
	weaponCooldownBonus = 0;
	childrenDestroyed = 0;
	projectileSpeedBonus = 0;
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
	enemy = true;
	allied = false;
	maxHP = 10;
	hp = 10.f;
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
	if (spawnGroup)
	{
		assert(false);
	}
	if (entity)
	{
		this->shipProperty->shouldDelete = true;
		this->shipProperty->sleeping = 0;
		this->shipProperty->ship = 0;
		shipProperty = 0;
	}
}

Random cooldownRand;
void Ship::RandomizeWeaponCooldowns()
{
//	if (!ai)
//		return;
	for (int i = 0; i < weapons.Size(); ++i)
	{
		Weapon * weap = weapons[i];
		weap->currCooldownMs = (int) (weap->cooldown.Milliseconds() * cooldownRand.Randf());
//		weap->lastShot = /*flyTime +*/ Time::Milliseconds(weap->cooldown.Milliseconds() * cooldownRand.Randf());
	}
}

List<Entity*> Ship::Spawn(ConstVec3fr atLocalPosition, Ship * in_parent)
{	

	std::cout<<"\nPossible kills: "<<++spaceShooter->LevelPossibleKills()->iValue;

	/// Reset stuffs if not already done so.
	movementDisabled = false;
	RandomizeWeaponCooldowns();

	Vector3f atPosition = atLocalPosition + levelEntity->worldPosition;
	atPosition.z = 0;
//	atPosition.y += levelEntity->worldPosition

	/// Stuff.
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
	
	PhysicsProperty * pp = new PhysicsProperty();
	entity->physics = pp;
	// Setup physics.
	pp->type = PhysicsType::DYNAMIC;
	float radians = PI / 2;
	if (enemy)
	{
		pp->collisionCategory = CC_ENEMY;
		pp->collisionFilter = CC_ALL_PLAYER;
		/// Turn to face left -X
		if (!parent)
			entity->SetRotation(Vector3f(radians, radians, 0));
	}
	/// Allied
	else 
	{
		pp->velocity = spaceShooter->level.BaseVelocity();
		pp->collisionCategory = CC_PLAYER;
		pp->collisionFilter = CC_ALL_ENEMY;
		/// Turn to face X+
		entity->SetRotation(Vector3f(radians, -radians, 0));
	}
	pp->collisionCallback = true;
	pp->shapeType = PhysicsShape::AABB;
	/// Adjust physics model as needed.
	if (this->physicsModel.Length())
	{
		if (this->physicsModel == "GraphicModel")
		{
			// Same model as in graphics.
			pp->shapeType = PhysicsShape::MESH;
		}
	}
	// By default, set invulerability on spawn.
	this->spawnInvulnerability = true;
	ShipProperty * sp = new ShipProperty(this, entity);
	shipProperty = sp;
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
		script->variables.Add(Movement::GetTypesAsVariables());
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
		if (enemy)
			activeLevel->enemyShips.AddItem(newShip);
		else 
			activeLevel->alliedShips.AddItem(newShip);
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

/// Despawns children. Does not resolve parent-pointers.
void Ship::Despawn(bool doExplodeEffectsForChildren)
{
	if (!spawned)
		return;
	for (int i = 0; i < children.Size(); ++i)
	{
		Ship * child = children[i];
		if (doExplodeEffectsForChildren)
			child->ExplodeEffects();
		child->parent = 0;
		child->Despawn(doExplodeEffectsForChildren);
	}
//	LogMain("Despawning ship "+name+" with children "+childrrr, INFO);
	spawned = false;

	// Delete entity first.
	Entity * tmp = entity;
	entity = 0;
	std::cout<<"\nDeleting entity "+tmp->name;
	MapMan.DeleteEntity(tmp);
	/// Waaaat.
	shipEntities.RemoveItem(tmp);

	/// Unbind link from property to this ship.
	shipProperty->sleeping = true; // Set sleeping so it shouldn't process anything anymore.
	shipProperty->ship = 0;

	/// Notify parent if child?
	if (parent)
	{
		++parent->childrenDestroyed;
		parent->children.RemoveItem(this);
		parent = 0;
	}

	children.Clear();
	if (spawnGroup)
	{
		if (hp <= 0)
			spawnGroup->OnShipDestroyed(this);
		else
			spawnGroup->OnShipDespawned(this);
	}
}

/// Checks current movement. Will only return true if movement is target based and destination is within threshold.
bool Ship::ArrivedAtDestination()
{
//	LogMain("Update maybe", INFO);
	return false;
}

void Ship::Process(int timeInMs)
{
	/// If destroyed from elsewhere..?
	if (entity == 0)
		return;
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
	if (movements.Size())
		movements[currentMovement].OnFrame(timeInMs);
	// AI
	ProcessAI(timeInMs);
	// Weapon systems.
	ProcessWeapons(timeInMs);
	// Shield
	if (hasShield)
	{
		// Repair shield
		shieldValue += timeInMs * shieldRegenRate * (activeSkill == POWER_SHIELD? 10.f : 0.001f);
		if (shieldValue > MaxShield())
			shieldValue = MaxShield();
		if (allied)
			spaceShooter->UpdateUIPlayerShield(false);
	}
	if (allied)
	{
		hp += timeInMs * armorRegenRate * 0.001f;
		if (hp > maxHP)
			hp = (float) maxHP;
		spaceShooter->UpdateUIPlayerHP(false);
	}
}

void Ship::ProcessAI(int timeInMs)
{
	// Don't process inactive ships..
	if (!enemy)
		return;
	if (rotations.Size() == 0)
		return;
	// Rotate accordingly.
	Rotation & rota = rotations[currentRotation];
	rota.OnFrame(timeInMs);
	// Increase time spent in this state accordingly.
	timeInCurrentRotation += timeInMs;
	if (timeInCurrentRotation > rota.durationMs && rota.durationMs > 0)
	{
		currentRotation = (currentRotation + 1) % rotations.Size();
		timeInCurrentRotation = 0;
		Rotation & rota2 = rotations[currentRotation];
		rota2.OnEnter(this);
	}
	if (!canMove)
		return;
	// Move?
	Entity * shipEntity = entity;
	Movement & move = movements[currentMovement];
	move.OnFrame(timeInMs);
	// Increase time spent in this state accordingly.
	timeInCurrentMovement += timeInMs;
	if (timeInCurrentMovement > move.durationMs && move.durationMs > 0)
	{
		currentMovement = (currentMovement + 1) % movements.Size();
		timeInCurrentMovement = 0;
		Movement & newMove = movements[currentMovement];
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

	// enemy AI fire all weapons simultaneously for the time being.
	if (enemy)
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
// Sets new bonus, updates weapons if needed.
void Ship::SetProjectileSpeedBonus(float newBonus)
{
	projectileSpeedBonus = newBonus;
} 
	
// Sets new bonus, updates weapons if needed.
void Ship::SetWeaponCooldownBonus(float newBonus)
{
	weaponCooldownBonus = newBonus;
} 


void Ship::SetWeaponCooldownByID(int id, int newcooldown)
{
	for (int i = 0; i < weapons.Size(); ++i)
	{
		Weapon * weap = weapons[i];
		if (weap->type == id)
			weap->cooldown = newcooldown;
	}
	for (int i = 0; i < children.Size(); ++i)
	{
		children[i]->SetWeaponCooldownByID(id, newcooldown);
	}
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
	if (movements.Size())
		movements[currentMovement].OnSpeedUpdated();
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

/// Returns true if destroyed -> shouldn't touch any more.
bool Ship::Damage(float amount, bool ignoreShield)
{
	if (allied && playerInvulnerability)
		return false;
	if (!enemy)
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
			return false;
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
	{
		Destroy();
		return true;
	}
	return false;
}

void Ship::Destroy()
{	
	if (destroyed)
		return;
	destroyed = 0;
	if (entity)
	{
		if (spawnGroup)
		{
			spawnGroup->OnShipDestroyed(this);
		}
		ShipProperty * sp = entity->GetProperty<ShipProperty>();
		if (sp)
			sp->sleeping = true;


		// Explosion?
		ExplodeEffects();
		// Increase score and kills.
		if (!allied)
		{
			spaceShooter->LevelScore()->iValue += this->score;
			std::cout<<"\nKills: "<<spaceShooter->LevelKills()->iValue++;
			spaceShooter->OnScoreUpdated();
		}
		else 
			failedToSurvive = true;
		/// Despawn.
		Despawn(true);
	}
}

/// GFX and SFX
void Ship::ExplodeEffects()
{
	// Add a temporary emitter to the particle system to add some sparks to the collision
	SparksEmitter * tmpEmitter;

	/// Make cool emitter that emits from vertices or faces of the model?
	if (boss)
	{
		std::cout<<"\nExploding @ "<<this->entity->worldPosition;
		List<Triangle> tris = entity->GetTris();
		tmpEmitter = new SparksEmitter(tris);
		tmpEmitter->newType = true;
		/// Set emitters?
		tmpEmitter->positionEmitter.type = EmitterType::TRIANGLES;
		tmpEmitter->velocityEmitter.type = EmitterType::PLANE_XY; // Random XY
		tmpEmitter->constantEmission = 3000;
		tmpEmitter->SetParticleLifeTime(3.5f);
		tmpEmitter->SetEmissionVelocity(3.5f);
	}
	else 
	{
		tmpEmitter = new SparksEmitter(entity->worldPosition);
		tmpEmitter->SetParticleLifeTime(2.5f);
		tmpEmitter->constantEmission = 750;
		tmpEmitter->SetEmissionVelocity(2.5f);
	}

	tmpEmitter->SetRatioRandomVelocity(1.0f);
	tmpEmitter->instantaneous = true;
	tmpEmitter->SetScale(0.1f);
	Vector4f color =  Vector4f(1,0.5f,0.1f,1.f);// entity->diffuseMap->averageColor;
	color.w *= 0.5f;
	tmpEmitter->SetColor(color);
	GraphicsMan.QueueMessage(new GMAttachParticleEmitter(tmpEmitter, sparks));
	/// SFX
	QueueAudio(new AMPlaySFX("sfx/Ship Death.wav"));
}


bool Ship::DisableWeaponsByID(int id)
{
	for (int i = 0; i < weapons.Size(); ++i)
	{
		Weapon * weap = weapons[i];
		if (weap->type == id)
			weap->enabled = false;
	}
	for (int i = 0; i < children.Size(); ++i)
		children[i]->DisableWeaponsByID(id);
	return true;
}

#define FOR_ALL_WEAPONS \
	for (int i = 0; i < weapons.Size(); ++i)\
	{\
		Weapon * weap = weapons[i];
bool Ship::EnableWeaponsByID(int id)
{
	FOR_ALL_WEAPONS
		if (weap->type == id)
			weap->enabled = true;
	}
	for (int i = 0; i < children.Size(); ++i)
		children[i]->EnableWeaponsByID(id);
	return true;
}

bool Ship::DisableAllWeapons()
{
	for (int i = 0; i < weapons.Size(); ++i)
	{
		Weapon * weap = weapons[i];
		weap->enabled = false;
	}
	for (int i = 0; i < children.Size(); ++i)
		children[i]->DisableAllWeapons();
	return true;
}


void Ship::SetMovement(Movement & movement)
{
	this->movements.Clear();
//		move.vec = Vector2f(-10.f, targetEntity->worldPosition.y); 
	this->movements.AddItem(movement);
	movements[0].OnEnter(this);
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
				*newWeap = *refWeap; // Weapon::Get(refWeap->type, refWeap->level);
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
	this->maxHP = armor.maxHP;
	hp = (float) maxHP;
	shieldValue = (float) (this->maxShieldValue = (float)shield.maxShield);
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
	if (!enemy)
	{
		QueueGraphics(new GMSetUIi("ActiveWeapon", GMUI::INTEGER_INPUT, index));
	}
	return true;
}

/// Calls OnEnter for the initial movement pattern.
void Ship::StartMovement()
{
	if (rotations.Size())
		rotations[0].OnEnter(this);
	if (movements.Size())
		movements[0].OnEnter(this);
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
