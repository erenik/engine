/// Emil Hedemalm
/// 2015-01-17
/// Building o.o

#include "MORPG/Character/Character.h"
#include "MathLib.h"

class Building;
class Model;
class Texture;


struct BuildingSlot 
{
	BuildingSlot();
	/// 
	Character * owner;
	// If for sale, price >= 0 if not for sale, price < 0
	int price;
	/// Building currently there.
	Building * building;

	/// Size dimensions in in-game XYZ. Mainly X and Z are interesting. Y for height.
	Vector3f size;
	/// 3D position or use grid position..?
	Vector3f position;
};

// Model, texture, characters living there. Characters currently in there (if in-door zone is wanted).
class Building 
{
public:
	Building(Model * model, Texture * texture);
	// Slot currently placed on.
	BuildingSlot * slot;
	Model * model;
	Texture * texture;
	List<Character*> residents;
	List<Character*> charactersInside;
};

