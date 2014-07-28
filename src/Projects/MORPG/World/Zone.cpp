/// Emil Hedemalm
/// 2014-07-27
/// A single zone within the world.

#include "Zone.h"

Zone::Zone()
{
	isWater = false;
	isMountain = false;
}

/// If this zone is to be painted on a map, what color would it be?
Vector4f Zone::GetColor()
{
	if (isWater)
		return Vector3f(0,0,1);
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
};


bool Zone::WriteTo(std::fstream & file)
{
	return true;
}
bool Zone::ReadFrom(std::fstream & file)
{
	return true;
}

