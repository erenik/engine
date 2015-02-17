#include "CompactPhysics.h"
#include "PhysicsProperty.h"
#include <fstream>

CompactPhysics::CompactPhysics(){
	Nullify();
}
CompactPhysics::~CompactPhysics(){}

/// Nullifies all relevant pointers and numbers
void CompactPhysics::Nullify(){
	type = NULL;
	physicsShape = NULL;
	physicsMesh = NULL;
	state = NULL;
	physicalRadius = NULL;
//	velocity
//	acceleration
#ifdef USE_MASS
	mass = 1.0f;
	inverseMass = 1.0f;
	volume = 1.0f;
	density = 1.0f;
#endif
	restitution = NULL;
	friction = NULL;
	/// See PhysicsProperty.h
	collissionCallback = NULL;
	collissionCallbackRequirementValue = NULL;
	collisionsEnabled = NULL;
	noCollisionResolutions = NULL;
}

CompactPhysics::CompactPhysics(PhysicsProperty * physicsProperty)
{
	Nullify();
	type = physicsProperty->type;
	physicsShape = physicsProperty->shapeType;
	state = physicsProperty->state;
	physicalRadius = physicsProperty->physicalRadius;
	velocity = physicsProperty->velocity;
	acceleration = physicsProperty->acceleration;
#ifdef USE_MASS
	mass = physicsProperty->mass;
	volume = physicsProperty->volume;
	density = physicsProperty->density;
#endif
	restitution = physicsProperty->restitution;
	friction = physicsProperty->friction;
	/// See PhysicsProperty.h
	collissionCallback = physicsProperty->collissionCallback;
	collissionCallbackRequirementValue = physicsProperty->collissionCallbackRequirementValue;
	collisionsEnabled = physicsProperty->collisionsEnabled;
	noCollisionResolutions = physicsProperty->noCollisionResolutions;
}

/// Compact Physics versions
#define CP_INITIAL_VERSION	0x00000000
#define CP_VERSION_1	0x00000010       // Begin first version here, since I was retarded not to use version in the beginning...
#define CP_VERSION_2    0x00000011       //  Add mass and stuff next!
#define CP_CURRENT_VERSION	CP_VERSION_1

/// Reads data from file stream
bool CompactPhysics::ReadFrom(std::fstream& file){

	int version = 0;

	file.read((char*) &version, sizeof(int));

	// Depending on version, do stuff.
	// For the first version, I was retarded and forgot version, so that means we actually read the type, so assign that and continue with just reading the physics shape.
	if (version < CP_VERSION_1){
		type = version;
		version = CP_INITIAL_VERSION;
	}
	else
		file.read((char*) &type, sizeof(int));

	file.read((char*) &physicsShape, sizeof(int));

	/// Simple checks..
	assert(type >= PhysicsType::STATIC && type < PhysicsType::NUM_TYPES);
	assert(physicsShape >= ShapeType::SPHERE && type < ShapeType::NUM_TYPES);

	file.read((char*) &state, sizeof(int));
	file.read((char*) &physicalRadius, sizeof(float));
	file.read((char*) &velocity, sizeof(Vector3f));
	file.read((char*) &acceleration, sizeof(Vector3f));
	file.read((char*) &type, sizeof(int));

	file.read((char*) &restitution, sizeof(float));
	file.read((char*) &friction, sizeof(float));

	/// In version 1, we added three new variables!
	if (version >= CP_VERSION_1){
		/// See PhysicsProperty.h
		file.read((char*) &collissionCallback, sizeof(int));
		file.read((char*) &collissionCallbackRequirementValue, sizeof(float));
		file.read((char*) &collisionsEnabled, sizeof(bool));
		file.read((char*) &noCollisionResolutions, sizeof(bool));
	}
	/// Set appropriate default values to said variables if they were not decalred earlier, since they will have null values for now..!
	else {
		collissionCallback = 0;
		collissionCallbackRequirementValue = 0;
		collisionsEnabled = true;
		noCollisionResolutions = false;
	}
    /// Add mass-volume, etc. in version 2!
	if (version >= CP_VERSION_2){
	    assert(false);
        file.read((char*) &mass, sizeof(float));
        file.read((char*) &volume, sizeof(float));
        file.read((char*) &density, sizeof(float));
    }
    else {
        mass = volume = density = 1.0f;
    }

	return true;
}

/// Write data to file stream
bool CompactPhysics::WriteTo(std::fstream& file){
	/// Simple checks..
	assert(type >= PhysicsType::STATIC && type < PhysicsType::NUM_TYPES);
	assert(physicsShape >= ShapeType::SPHERE && type < ShapeType::NUM_TYPES);

	/// First write a version. Always fucking write a version.
	int version = CP_CURRENT_VERSION;
	file.write((char*) &version, sizeof(int));

	/// Write initial version data.
	file.write((char*) &type, sizeof(int));
	file.write((char*) &physicsShape, sizeof(int));
	file.write((char*) &state, sizeof(int));
	file.write((char*) &physicalRadius, sizeof(float));
	file.write((char*) &velocity, sizeof(Vector3f));
	file.write((char*) &acceleration, sizeof(Vector3f));
	file.write((char*) &type, sizeof(int));

	file.write((char*) &restitution, sizeof(float));
	file.write((char*) &friction, sizeof(float));

	/// In version 1, we added four new variables!
	if (version >= CP_VERSION_1){
		/// See PhysicsProperty.h
		file.write((char*) &collissionCallback, sizeof(int));
		file.write((char*) &collissionCallbackRequirementValue, sizeof(float));
		file.write((char*) &collisionsEnabled, sizeof(bool));
		file.write((char*) &noCollisionResolutions, sizeof(bool));
	}
	else if (version >= CP_VERSION_2){
	#ifdef USE_MASS
        file.write((char*) &mass, sizeof(float));
        file.write((char*) &volume, sizeof(float));
        file.write((char*) &density, sizeof(float));
    #endif
	}

	return true;
}

/// Calculates size the entity will take when saving, including any eventual sub-properties (if we place them here?)
int CompactPhysics::Size(){
	// No pointers in this object, so cheerio!
	return sizeof(CompactPhysics);
}
