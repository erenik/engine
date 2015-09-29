/// Emil Hedemalm
/// 2015-01-21
/// Weapon..

#include "SpaceShooter2D.h"

#include "File/File.h"
#include "File/LogFile.h"
#include "String/StringUtil.h"

#include "TextureManager.h"

#include "Model/ModelManager.h"

#include "Entity/EntityManager.h"
#include "Entity/Entity.h"

WeaponSet::WeaponSet()
{

}
WeaponSet::~WeaponSet()
{
	ClearAndDelete();
}
WeaponSet::WeaponSet(WeaponSet & otherWeaponSet)
{
	// Create duplicates of all weapons!
	for (int i = 0; i < otherWeaponSet.Size(); ++i)
	{
		Weapon * weap = new Weapon(*otherWeaponSet[i]);
		AddItem(weap);
	}
}

Weapon::Weapon()
{
	explosionRadius = 0;
	penetration = 0;
	burst = false;
	aim = false;
	estimatePosition = false;
	projectilePath = STRAIGHT;
	burstStart = lastShot = Time(TimeType::MILLISECONDS_NO_CALENDER, 0);
	cooldown = Time(TimeType::MILLISECONDS_NO_CALENDER, 1000);
	damage = 5;
	angle = 0;
	homingFactor = 0;
	projectileSpeed = 5.f;
	arcDelay = 1000;

	burstRounds = 3;
	burstRoundsShot = 0;
	burstRoundDelay = Time(TimeType::MILLISECONDS_NO_CALENDER, 50);
}

bool Weapon::Get(String byName, Weapon * weapon)
{
	for (int i = 0; i < types.Size(); ++i)
	{
		Weapon & weap = types[i];
		if (weap.name == byName)
		{
			*weapon = weap;
			return true;
		}
	}
	return false;
}

Weapon Weapon::Get(int type, int level)
{
	for (int i = 0; i < types.Size(); ++i)
	{
		Weapon & weap = types[i];
		if (weap.type == type && weap.level == level)
		{
			return weap;
		}
	}
	assert(false);
	return Weapon();
}


bool Weapon::LoadTypes(String fromFile)
{
	List<String> lines = File::GetLines(fromFile);
	if (lines.Size() == 0)
		return false;

	String separator;
	/// Column-names. Parse from the first line.
	List<String> columns;
	String firstLine = lines[0];
	// Keep empty strings or all will break.
	char tokenizer = '\t';
	columns = TokenizeCSV(firstLine, tokenizer);
	LogMain("Loading weapons from file: "+fromFile, INFO);

	// For each line after the first one, parse data.
	for (int j = 1; j < lines.Size(); ++j)
	{
		String & line = lines[j];
		// Keep empty strings or all will break.
		List<String> values = TokenizeCSV(line, tokenizer);
		// If not, now loop through the words, parsing them according to the column name.
		// First create the new spell to load the data into!
		Weapon weapon;
		for (int k = 0; k < values.Size(); ++k)
		{
			String column;
			bool error = false;
			/// In-case extra data is added beyond the columns defined above..?
			if (columns.Size() > k)
				column = columns[k];
			String value = values[k];
			column.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (column == "Weapon Name")
				weapon.name = value;
			else if (column == "Type")
				weapon.type = value.ParseInt();
			else if (column == "ShootSFX")
				weapon.shootSFX = value;
			else if (column == "HitSFX")
				weapon.hitSFX = value;
			else if (column == "Level")
				weapon.level = value.ParseInt();
			else if (column == "Explosion Radius")
				weapon.explosionRadius = value.ParseFloat();
			else if (column == "Cooldown")
				weapon.cooldown = Time::Milliseconds(value.ParseFloat());
			else if (column == "Penetration")
				weapon.penetration = value.ParseFloat();
			else if (column == "Burst")
				weapon.burst = value.ParseBool();
			else if (column == "Burst details")
			{
				List<String> tokens = value.Tokenize(",");
				if (tokens.Size() >= 2)
				{
					weapon.burstRoundDelay = Time::Milliseconds(tokens[0].ParseInt());
					weapon.burstRounds = tokens[1].ParseInt();
				}
			}
			else if (column == "HomingFactor")
				weapon.homingFactor = value.ParseFloat();
			else if (column == "Angle")
			{
				if (value.Contains("Aim"))
					weapon.aim = true;
				else if (value.Contains("Predict"))
					weapon.estimatePosition = true;
				weapon.angle = value.ParseInt();
			}
			else if (column == "Projectile path")
			{
				if (value == "Homing")
					weapon.projectilePath = HOMING;
				else if (value == "Spinning outward")
					weapon.projectilePath = SPINNING_OUTWARD;
			}
			else if (column == "Projectile speed")
				weapon.projectileSpeed = value.ParseFloat();
			else if (column == "Damage")
				weapon.damage = value.ParseInt();
			else if (column == "Abilities")
			{
				// la...
			}
			else if (column == "Ability Trigger")
			{
				// lall..
			}
			else if (column == "Projectile Shape")
				weapon.projectileShape = value;
			else if (column == "Projectile Scale")
				weapon.projectileScale = value.ParseFloat();
			else if (column == "Max Range")
				weapon.maxRange = value.ParseFloat();
			else if (column == "Arc Delay")
				weapon.arcDelay = value.ParseFloat();
			else if (column == "Max Bounces")
				weapon.maxBounces = value.ParseFloat();
			else 
			{
		//		std::cout<<"\nUnknown column D:";
			}
			if (error)
			{
				std::cout<<"\n .. when parsing line \'"<<line<<"\'";
			}
		}
		// Check for pre-existing ship of same name, remove it if so.
		for (int i = 0; i < types.Size(); ++i)
		{
			Weapon & type = types[i];
			if (type.name == weapon.name)
			{
				types.RemoveIndex(i);
				--i;
			}
		}
		LogMain("Weapon loaded: "+weapon.name, INFO);
		types.Add(weapon);
	}
	return true;
}

Random shootRand;

/// Moves the aim of this weapon turrent.
void Weapon::Aim(Ship * ship)
{
	// If no aim, just align it with the ship?
	if (!aim)
		return;

	Entity * target = NULL;
	// Aim.
	if (ship->allied)
	{
		assert(false && "Implement. Press ignore to continue anyway.");
	}
	else 
	{
		target = playerShip->entity;
	}
	if (target == NULL)
		return;
	Entity * shipEntity = ship->entity;
	// Estimate position upon impact?
	Vector3f targetPos = target->position;
	Vector3f toTarget = targetPos - shipEntity->position;
	if (estimatePosition)
	{
		float dist = toTarget.Length();
		// Check velocity of target.
		Vector3f vel = target->Velocity();
		// Estimated position upon impact... wat.
		float seconds = dist / projectileSpeed;
		Vector3f estimatedPosition = targetPos + vel * seconds;
		toTarget = estimatedPosition - shipEntity->position;
	}
	// Aim at the player.
	currentAim = toTarget.NormalizedCopy();
}

/// Shoots using previously calculated aim.
void Weapon::Shoot(Ship * ship)
{
	float firingSpeedDivisor = ship->activeSkill == ATTACK_FRENZY? 0.4f : 1.f; 
	/// For burst..
	if (burst)
	{
		if (burstRoundsShot < burstRounds)
		{
			// Check time between burst rounds.
			if (flyTime < burstRoundDelay * firingSpeedDivisor + lastShot)
				return;
			++burstRoundsShot;
		}
		else {
			if (flyTime < cooldown * firingSpeedDivisor + burstStart)
				return;
			burstStart = flyTime;
			burstRoundsShot = 0;
			++burstRoundsShot;
		}
	}
	// Regular fire
	else {
		if (flyTime < cooldown * firingSpeedDivisor + lastShot)
			return;
	}
	// Shoot!
	if (type ==	LIGHTNING)
	{
		/// Create a lightning storm...!
		ProcessLightning(ship, true);
		lastShot = flyTime;
		return;
	}

	Entity * shipEntity = ship->entity;
	Color color;
	if (ship->allied)
		color = Vector4f(1.f, 0.5f, .1f, 1.f);
	else
		color = Vector4f(0.8f,0.7f,0.1f,1.f);
	Texture * tex = TexMan.GetTextureByColor(color);
	// Grab model.
	Model * model = ModelMan.GetModel("obj/Proj/"+projectileShape);
	if (!model)
		model = ModelMan.GetModel("sphere.obj");

	Entity * projectileEntity = EntityMan.CreateEntity(name + " Projectile", model, tex);
	ProjectileProperty * projProp = new ProjectileProperty(*this, projectileEntity);
	projectileEntity->properties.Add(projProp);
	// Set scale and position.
	projectileEntity->position = shipEntity->position;
	projectileEntity->SetScale(Vector3f(1,1,1) * projectileScale);
	projProp->color = color;
	projectileEntity->RecalculateMatrix();
	// pew
	Vector3f dir(-1.f,0,0);
	// For defaults of forward, invert for player
	if (ship->allied)
		dir *= -1.f;
	if (aim)
	{
		dir = currentAim;
	}
	// Angle, +180
	else if (angle)
	{
		/// Grab current forward-vector.
		Vector3f forwardDir = shipEntity->transformationMatrix.Product(Vector4f(0,0,-1,1)).NormalizedCopy();
		/// Get angles from current dir.
		float angleDegrees = GetAngled(forwardDir.x, forwardDir.y);

		float worldAngle = DEGREES_TO_RADIANS((float)angleDegrees + 180 + angle);
		dir[0] = cos(worldAngle);
		dir[1] = sin(worldAngle);
	}
	/// Change initial direction based on stability of the weapon?
	float stability = 0.99f;
	if (ship->activeSkill == ATTACK_FRENZY)
		stability *= 0.75f;
	// Get orthogonal direction.
	Vector3f dirRight = dir.CrossProduct(Vector3f(0,0,1));
	if (stability < 1.f)
	{
		float amplitude = 1 - stability;
		float randomEffect = shootRand.Randf(amplitude * 2.f) - amplitude;
		dir = dir + dirRight * randomEffect;
		dir.Normalize();
	}
	projProp->direction = dir; // For autoaim initial direction.

	Vector3f vel = dir * projectileSpeed;
	PhysicsProperty * pp = projectileEntity->physics = new PhysicsProperty();
	pp->type = PhysicsType::DYNAMIC;
	pp->velocity = vel;
	pp->collisionCallback = true;	
	pp->maxCallbacks = -1; // unlimited callbacks or penetrating projs won't work
	pp->faceVelocityDirection = true;
	// Set collision category and filter.
	pp->collisionCategory = ship->allied? CC_PLAYER_PROJ : CC_ENEMY_PROJ;
	pp->collisionFilter = ship->allied? CC_ENEMY : CC_PLAYER;
	// Add to map.
	MapMan.AddEntity(projectileEntity);
	projectileEntities.Add(projectileEntity);
	lastShot = flyTime;

	// Play sfx
	QueueAudio(new AMPlaySFX("sfx/"+shootSFX+".wav", 1.f));
}

/// Called to update the various states of the weapon, such as reload time, making lightning arcs jump, etc.
void Weapon::Process(Ship * ship)
{
	if (type == LIGHTNING && arcs.Size())
	{
		ProcessLightning(ship);
	}
}

LightningArc::LightningArc()
{
	graphicalEntity = 0;
	arcFinished = false;
	child = 0;
	damage = -1;
	maxBounces = -1;
}

void Weapon::ProcessLightning(Ship * owner, bool initial /* = true*/)
{
	if (initial)
	{
		// Create a dummy arc?
		LightningArc * arc = new LightningArc();
		arc->position = owner->entity->position;
		arc->maxRange = maxRange;
		arc->damage = damage;
		arc->arcTime = flyTime;
		arc->maxBounces = maxBounces;
		arcs.AddItem(arc);
		shipsStruckThisArc.Clear();
	}
	// Proceed all arcs which have already begun.
	bool arced = false;
	bool arcingAllDone = true;
	for (int i = 0; i < arcs.Size(); ++i)
	{
		// Create arc to target entity.
		LightningArc * arc = arcs[i];
		if (arc->arcFinished)
			continue;
		arcingAllDone = false;
		if ((flyTime - arc->arcTime).Milliseconds() < arcDelay)
			continue;

		// Find next entity.
		List<float> distances;
		List<Ship*> possibleTargets = spaceShooter->level.GetShipsAtPoint(arc->position, maxRange, distances);
		std::cout<<"\nPossible targets: "<<possibleTargets.Size();
		possibleTargets.RemoveUnsorted(shipsStruckThisArc);
		std::cout<<" - shipsAlreadyStruck("<<shipsStruckThisArc.Size()<<") = "<<possibleTargets.Size();
		if (possibleTargets.Size() == 0)
		{
			/// Unable to Arc, set the child to a bad number?
			arc->arcFinished = true;
			continue;
		}
		// Grab first one which hasn't already been struck?
		Ship * target = possibleTargets[0];
		// Recalculate distance since list was unsorted earlier...
		float distance = (target->entity->position - arc->position).Length();
		/// Grab closest one.
		for (int j = 1; j < possibleTargets.Size(); ++j)
		{
			Ship * t2 = possibleTargets[j];
			float d2 = (t2->entity->position - arc->position).Length();
			if (d2 < distance)
			{
				target = t2;
				distance = d2;
			}
		}
		if (distance > arc->maxRange)
		{
			arc->arcFinished = true;
			continue;
		}

		LightningArc * newArc = new LightningArc();
		newArc->position = target->entity->position;
		newArc->maxRange = arc->maxRange - distance;
		newArc->damage = arc->damage * 0.8;
		newArc->maxBounces = arc->maxBounces - 1;
		if (newArc->maxBounces <= 0)
			newArc->arcFinished = true;
		arc->child = newArc;
		arc->arcFinished = true;
		newArc->arcTime = flyTime;
		arcs.AddItem(newArc);
		// Pew-pew it!
		shipsStruckThisArc.AddItem(target);
		assert(shipsStruckThisArc.Duplicates() == 0);
		std::cout<<"\nThunderstruck! "<<target->entity->position;
		target->Damage(arc->damage, false);
		/// Span up a nice graphical entity to represent the bolt
		Entity * entity = EntityMan.CreateEntity("BoldPart", ModelMan.GetModel("cube.obj"), TexMan.GetTexture("0x00FFFF"));
		entity->position = (arc->position + newArc->position) * 0.5f;
		/// Rotate it accordingly.
		Vector3f direction = newArc->position - arc->position;
		direction.Normalize();
		entity->rotation.z = Angle(direction.x, direction.y).Radians();
		/// Scale it.
		entity->scale = Vector3f(distance, 0.1f, 0);
		entity->RecalculateMatrix();
		MapMan.AddEntity(entity, true, false);
		newArc->graphicalEntity = entity;
	}
	//	std::cout<<"\nLightning Clean-up";
	for (int i = 0; i < arcs.Size(); ++i)
	{
		LightningArc * arc = arcs[i];
		if ((flyTime - arc->arcTime).Milliseconds() > arcDelay * 2)
		{
			if (arc->graphicalEntity)
				MapMan.DeleteEntity(arc->graphicalEntity);
			arcs.RemoveItem(arc);
			--i;
			delete arc;
		}
	}
}
