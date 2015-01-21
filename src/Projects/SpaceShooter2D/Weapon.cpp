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
	projectilePath = STRAIGHT;
	lastShotMs = 0;
	cooldownMs = 1000;
	damage = 5;
	angle = 0;
	projectileSpeed = 5.f;
}

Weapon Weapon::Get(String byName)
{
	for (int i = 0; i < types.Size(); ++i)
	{
		Weapon & weap = types[i];
		if (weap.name == byName)
			return weap;
	}
	std::cout<<"\nERROR: Unable to find weapon by name \'"<<byName<<"\'";
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
				if (value == "Aim")
					weapon.aim = true;
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


void Weapon::Shoot(Ship * ship)
{
	Entity * shipEntity = ship->entity;
	// Shoot.
	Texture * tex = TexMan.GetTextureByColor(Color(0,255,255,255));
	Entity * projectileEntity = EntityMan.CreateEntity(name + " Projectile", ModelMan.GetModel("sphere.obj"), tex);
	ProjectileProperty * pp = new ProjectileProperty(*this, projectileEntity);
	projectileEntity->properties.Add(pp);
	// Set scale and position.
	projectileEntity->position = shipEntity->position;
	projectileEntity->scale *= 0.1f;
	projectileEntity->RecalculateMatrix();
	// pew
	Vector3f dir(-1.f,0,0);
	if (aim)
	{
		// Aim.
	}
	// Angle, +180
	else if (angle)
	{
		float worldAngle = (float)angle + 180;
		dir.x = cos(worldAngle);
		dir.y = sin(worldAngle);
	}
	if (ship->allied)
		dir *= -1.f;
	Vector3f vel = dir * projectileSpeed;
	projectileEntity->physics = new PhysicsProperty();
	projectileEntity->physics->type = PhysicsType::DYNAMIC;
	projectileEntity->physics->velocity = vel;
	// Set collision category and filter.
	projectileEntity->physics->collisionCategory = ship->allied? CC_PLAYER : CC_ENEMY;
	projectileEntity->physics->collisionFilter = ship->allied? CC_ENEMY : CC_PLAYER;
	// Add to map.
	MapMan.AddEntity(projectileEntity);
	spaceShooter->projectileEntities.Add(projectileEntity);
	lastShotMs = nowMs;
}