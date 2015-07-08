/// Emil Hedemalm
/// 2015-01-21
/// Weapon..

#include "SpaceShooter2D.h"

#include "File/File.h"
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
	columns = TokenizeCSV(firstLine, ';');

	// For each line after the first one, parse data.
	for (int j = 1; j < lines.Size(); ++j)
	{
		String & line = lines[j];
		// Keep empty strings or all will break.
		List<String> values = TokenizeCSV(line, ';');
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
			else if (column == "Level")
				weapon.level = value.ParseInt();
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
	/// Initialize the weapon as if it had just been fired at a random time (AI only)
	if (lastShot.Milliseconds() == 0 && ship->ai)
	{
		lastShot = flyTime - cooldown + Time(TimeType::MILLISECONDS_NO_CALENDER, 1000 + shootRand.Randf(cooldown.Milliseconds()));
		return;
	}
	/// For burst..
	if (burst)
	{
		if (burstRoundsShot < burstRounds)
		{
			// Check time between burst rounds.
			Time diff = flyTime - lastShot;
			if (diff < burstRoundDelay * firingSpeedDivisor)
				return;
			++burstRoundsShot;
		}
		else {
			Time diff = flyTime - burstStart;
			if (diff < cooldown * firingSpeedDivisor)
				return;
			burstStart = flyTime;
			burstRoundsShot = 0;
			++burstRoundsShot;
		}
	}
	// Regular fire
	else {
		Time diff = flyTime - lastShot;
		if (diff < cooldown * firingSpeedDivisor)
			return;
	}

	Entity * shipEntity = ship->entity;
	// Shoot.
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
	projectileEntity->SetScale(Vector3f(1,1,1));
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
}