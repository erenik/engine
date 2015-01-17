/// Emil Hedemalm
/// 2015-01-17
/// Building o.o

#include "Building.h"

Building::Building(Model * model, Texture * texture)
: model(model), texture(texture)
{
	slot = NULL;
}


BuildingSlot::BuildingSlot()
{
	owner = NULL;
	building = NULL;
}
