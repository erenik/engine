/// Emil Hedemalm
/// 2015-02-19
/// Class for handling the buildings in the game.

#ifndef TIFS_BUILDING_H
#define TIFS_BUILDING_H

class Entity;
class Model;
class Texture;

#include "MathLib/Vector3f.h"
#include "String/AEString.h"
#include "Random/Random.h"

class TIFSBuilding 
{
public:
	TIFSBuilding();
	/// Creates a building given a base name, and sets base, top, and texture sources based on it, and appending _*.ext 
	TIFSBuilding(String baseName);
	void Nullify();
	/// Creates it.
	static List<Entity*> CreateNew(ConstVec3fr atLocation, ConstVec3fr withGivenMaxSize);
	
	static bool LoadTypes(String fromSource = "data/BuildingTypes.txt");
	static List<TIFSBuilding*> types;
	static Random buildingRandom;

	String name;
private:
	static TIFSBuilding * GetType(ConstVec3fr forMaxSize, Vector3f & rotationNeeded);
	bool FetchResources();
	
	//// o.o
	String bottomSource, topSource, diffuseSource, specularSource, normalSource;
	/// o.o
	float floorHeight;
	/// Scale multiplied by default to the model, as most models were initially supplied in 1 unit heights, requiring scaling of the whole mesh.
	Vector3f scale;
	/// AABB?
	Model * bottomModel, * topModel;
	Texture * diffuseMap, * specularMap, * normalMap;
};

#endif