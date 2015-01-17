/// Emil Hedemalm
/// 2014-07-27
/// A single zone within the world.

#include "Zone.h"

#include "Entity/CompactEntity.h"

Zone::Zone()
{
	Nullify();
}

void Zone::Nullify()
{
	isWater = false;
	isMountain = false;
	elevation = 1.f;
}

/// Takes all models this zone is composed of and creates it for you. Will also create all characters within (hopefully including you!)
void Zone::CreateEntities()
{
	// Deletes and re-creates entities as needed.
	LoadFromCompactData();
}

/// If this zone is to be painted on a map, what color would it be?
Vector4f Zone::GetColor()
{
	/// o.o 0 = water-line, 1 = land, 0.1 to 0.9 = Beach?, 2 = hills, 3+ = mountains
	if (elevation <= -1.f)
	{
		return Vector3f(0,0, elevation * 0.5f + 1.f);
	}
	/// Shallow waters
	else if (elevation < 0.f)
	{
		// Lagoons?
		if (elevation > -0.1f)
			return Vector3f(0,1,1);
		// Regular waters.
		return Vector3f(0.2f, 0.5f, 1.f) * (1 + elevation) + Vector3f(0,0,1);
	}
	// Beach.
	else if (elevation < 0.5f)
	{
		return Vector3f(1.f, 1.f, 0.f);
	}
	// Grass to hills?
	else if (elevation < 2.f)
	{
		return Vector3f(0,1.f,0.f);
	}
	/// Hills to Mountains o.o
	else if (elevation < 3.f)
		return Vector3f(1,1,1) * 0.5f;
	// Mountains and snowy peaks!
	else 
	{
		return Vector3f(1,1,1);
	}
	return Vector3f(0,1,0);
}

bool Zone::IsWater()
{
	return isWater;
};

bool Zone::IsMountain()
{
	return isMountain;
}

void Zone::SetWater(bool w)
{ 
	isWater = w;
	elevation = -2.f;
};

void Zone::SetMountain(bool m)
{
	isMountain = m;
	elevation = 3.f;
}



bool Zone::WriteTo(std::fstream & file)
{
	return true;
}
bool Zone::ReadFrom(std::fstream & file)
{
	return true;
}

