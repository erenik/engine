/// Emil Hedemalm
/// 2014-02-25
/// An object that can be placed on a Grid. Contains references to all tiles which it occupies or interact with in some way.

#include "GridObject.h"
#include <fstream>
#include "FilePath/FilePath.h"

GridObjectType::GridObjectType()
{
	/// Set to invalid id.
	id = -1;
	texture = NULL;
}

GridObjectType::~GridObjectType()
{
	passability.ClearAndDelete();
}

/// Resizes the passability matrix as needed.
void GridObjectType::UpdatePassabilityMatrix()
{
	if (matrixSize == size)
		return;
	passability.ClearAndDelete();
	int num = size.x * size.y;
	for (int i = 0; i < num; ++i){
		passability.Add(new bool(false));
	}
}
/// Updates pivot-position depending on which tiles are marked as passable or not!
void GridObjectType::UpdatePivotPosition()
{
	Vector2i min, max;
	bool firstFound = false;
	for (int y = 0; y < size.y; ++y){
		for (int x = 0; x < size.x; ++x){
			bool * passable = passability[y * size.x + x];
			if (! *passable)
			{
				if (!firstFound){
					min = max = Vector2i(x,y);
					firstFound = true;
				}
				else {
					if (x < min.x)
						min.x = x;
					if (x > max.x)
						max.x = x;
					if (y < min.y)
						min.y = y;
					if (y > max.y)
						max.y = y;
				}
			}
		}
	}
	Vector2f center = Vector2f(size.x / 2.0f, size.y / 2.0f);
	pivotPosition = max + min + Vector2i(1,1);
	pivotPosition /= 2.0f;
	pivotPosition.y = size.y - pivotPosition.y;

	UpdatePivotDistances();
}


bool GridObjectType::WriteTo(std::fstream & file)
{
	/**  Versions
		0 - no id
		1 - id added
		2 - convert from pivotposition as Vector3f to Vector2f
	*/
	int version = 2;
	file.write((char*) &version, sizeof(int));
	file.write((char*) &id, sizeof(int));
	name.WriteTo(file);
	size.WriteTo(file);
	// Make sure ALL paths are relative.
	textureSource = FilePath::MakeRelative(textureSource);
	textureSource.WriteTo(file);
	int passabilityValues = passability.Size();
	file.write((char*) &passabilityValues, sizeof(int));
	for (int i = 0; i < passability.Size(); ++i){
		bool * value = passability[i];
		file.write((char*) value, sizeof(bool));
	}
	pivotPosition.WriteTo(file);
	return true;
}
bool GridObjectType::ReadFrom(std::fstream & file)
{
	passability.Clear();
	int version = 0;
	file.read((char*) &version, sizeof(int));
	if (version > 2)
		return false;
	/**  Versions
		0 - no id
		1 - id added
		2 - convert from pivotposition as Vector3f to Vector2f
	*/
	if (version == 0)
	{
		/// No id here, assign it automatically.
		id = -1;
	}
	else if (version >= 1){
		file.read((char*) &id, sizeof(int));
	}
	name.ReadFrom(file);
	size.ReadFrom(file);
	textureSource.ReadFrom(file);
	int passabilityValues;
	file.read((char*) &passabilityValues, sizeof(int));
	for (int i = 0; i < passabilityValues; ++i){
		bool value;
		file.read((char*) &value, sizeof(bool));
		passability.Add(new bool(value));
	}
	if (version < 2){
		Vector3f pos;
		pos.ReadFrom(file);
		pivotPosition = pos;
	}
	else {
		pivotPosition.ReadFrom(file);
	}
	UpdatePivotDistances();
	return true;	
}
/// Updates the 4 below. Call after changing pivot position or size.
void GridObjectType::UpdatePivotDistances(){
	/// Distances from pivot to the edges, to be used for rendering etc.
	pivotToLeft = -pivotPosition.x;
	pivotToRight = size.x - pivotPosition.x;
	pivotToTop = size.y - pivotPosition.y;
	pivotToBottom = -pivotPosition.y;
}

/////////////////////////// Object
GridObject::GridObject(GridObjectType * type)
{
	this->type = type;
}
GridObject::~GridObject()
{

}

/** Grid object versions
	0 - Initial, 2014-02-27
*/
bool GridObject::WriteTo(std::fstream & file)
{
	
	int version = 0;
	/// Write version
	file.write((char*)&version, sizeof(int));
	name.WriteTo(file);
	/// Don't save entity, as that should be generated after loading, it at all.
	/// Save type name to file, should be enough? Will have to make sure that no two types have the same name then though!
	assert(type);
	if (type)
		typeName = type->name;
	typeName.WriteTo(file);
	position.WriteTo(file);
	return true;
}
bool GridObject::ReadFrom(std::fstream & file)
{
	int version;
	file.read((char*)&version, sizeof(int));
	if (version != 0)
		return false;
	name.ReadFrom(file);
	typeName.ReadFrom(file);
	position.ReadFrom(file);
	return true;
}


//////////////////////////// Manager
GridObjectTypeManager * GridObjectTypeManager::gridObjectTypeManager = NULL;

GridObjectTypeManager::GridObjectTypeManager()
{
	
}

GridObjectTypeManager::~GridObjectTypeManager()
{
	objectTypes.ClearAndDelete();
}

void GridObjectTypeManager::Allocate()
{
	assert(gridObjectTypeManager == NULL);
	gridObjectTypeManager = new GridObjectTypeManager();
}

void GridObjectTypeManager::Deallocate()
{
	assert(gridObjectTypeManager);
	delete gridObjectTypeManager;
}

GridObjectTypeManager * GridObjectTypeManager::Instance()
{
	assert(gridObjectTypeManager);
	return gridObjectTypeManager;
}




GridObjectType * GridObjectTypeManager::New()
{
	GridObjectType * go = new GridObjectType();
	AssignID(go);
	objectTypes.Add(go);
	return go;
}
/// Sets path to the file to which we save/laod.
void GridObjectTypeManager::SetSavePath(String path)
{
	savePath = path;
}
	

/// Saves all types.
bool GridObjectTypeManager::Save()
{
	std::fstream file;
	file.open(savePath.c_str(), std::ios_base::out | std::ios_base::binary);
	if (!file.is_open())
		return false;

	int version = 0;
	file.write((char*) &version, sizeof(int));
	int types = objectTypes.Size();
	file.write((char*) &types, sizeof(int));
	for (int i = 0; i < types; ++i)
	{
		GridObjectType * go = objectTypes[i];
		go->WriteTo(file);
	}
	file.close();
	return true;
}
bool GridObjectTypeManager::Load()
{
	objectTypes.ClearAndDelete();

	std::fstream file;
	file.open(savePath.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
		return false;

	int version = 0;
	file.read((char*) &version, sizeof(int));
	if (version != 0)
		return false;
	int types = objectTypes.Size();
	file.read((char*) &types, sizeof(int));
	for (int i = 0; i < types; ++i)
	{
		GridObjectType * go = New();
		go->ReadFrom(file);
		if (go->id == -1)
			AssignID(go);
	}
	file.close();
	return true;
}

GridObjectType * GridObjectTypeManager::GetTypeByID(int id)
{
	for (int i = 0; i < objectTypes.Size(); ++i){	
		GridObjectType * t = objectTypes[i];
		if (t->id == id){
			return t;	
		}
	}
	return NULL;
}

GridObjectType * GridObjectTypeManager::GetType(String byName)
{
	for (int i = 0; i < objectTypes.Size(); ++i){	
		GridObjectType * t = objectTypes[i];
		if (t->name == byName){
			return t;	
		}
	}
	return NULL;
}

void GridObjectTypeManager::AssignID(GridObjectType* toType)
{
	for (int id = 0; true; id++){
		bool bad = false;
		for (int i = 0; i < objectTypes.Size(); ++i){	
			GridObjectType * t = objectTypes[i];
			if (t->id == id){
				bad = true;
				break;
			}
		}
		if (bad)
			continue;
		/// Good id.
		toType->id = id;
		return;
	}
	assert(false && "Failed to assign ID to type");
}