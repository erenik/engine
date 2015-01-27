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

Weapon::Weapon()
{
	burst = false;
	aim = false;
	estimatePosition = false;
	projectilePath = STRAIGHT;
	lastShotMs = 0;
	cooldownMs = 1000;
	damage = 5;
	angle = 0;
	projectileSpeed = 5.f;

	burstRounds = 3;
	burstRoundsShot = 0;
	burstRoundDelayMs = 50;
}

bool Weapon::Get(String byName, Weapon & weapon)
{
	for (int i = 0; i < types.Size(); ++i)
	{
		Weapon & weap = types[i];
		if (weap.name == byName)
		{
			weapon = weap;
			return true;
		}
	}
	return false;
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
	columns = TokenizeCSV(firstLine);

	// For each line after the first one, parse data.
	for (int j = 1; j < lines.Size(); ++j)
	{
		String & line = lines[j];
		// Keep empty strings or all will break.
		List<String> values = TokenizeCSV(line);
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
				weapon.projectileSpeed = value.ParseFloat() * 0.2f;
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
		target = spaceShooter->playerShip.entity;
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
	/// Initialize the weapon as if it had just been fired.
	if (lastShotMs == 0)
	{
		lastShotMs = nowMs - cooldownMs + 1000 + shootRand.Randf(cooldownMs);
		return;
	}
	/// For burst..
	if (burst)
	{
		if (burstRoundsShot < burstRounds)
		{
			// Check time between burst rounds.
			int timeDiff = nowMs - lastShotMs;
			if (timeDiff < burstRoundDelayMs)
				return;
			++burstRoundsShot;
		}
		else {
			int timeDiff = nowMs - burstStartMs;
			if (timeDiff < cooldownMs)
				return;
			burstStartMs = nowMs;
			burstRoundsShot = 0;
			++burstRoundsShot;
		}
	}
	// Regular fire
	else {
		int timeDiff = nowMs - lastShotMs;
		if (timeDiff < cooldownMs)
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
	Entity * projectileEntity = EntityMan.CreateEntity(name + " Projectile", ModelMan.GetModel("sphere.obj"), tex);
	ProjectileProperty * projProp = new ProjectileProperty(*this, projectileEntity);
	projectileEntity->properties.Add(projProp);
	// Set scale and position.
	projectileEntity->position = shipEntity->position;
	projectileEntity->scale *= 0.1f;
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
	Vector3f vel = dir * projectileSpeed;
	PhysicsProperty * pp = projectileEntity->physics = new PhysicsProperty();
	pp->type = PhysicsType::DYNAMIC;
	pp->velocity = vel;
	pp->collissionCallback = true;	
	pp->maxCallbacks = 1;
	// Set collision category and filter.
	pp->collisionCategory = ship->allied? CC_PLAYER_PROJ : CC_ENEMY_PROJ;
	pp->collisionFilter = ship->allied? CC_ENEMY : CC_PLAYER;
	// Add to map.
	MapMan.AddEntity(projectileEntity);
	spaceShooter->projectileEntities.Add(projectileEntity);
	lastShotMs = nowMs;
}