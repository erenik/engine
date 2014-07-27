/// Emil Hedemalm
/// 2014-07-27
/// Class encompassing an entire world.

#include "World.h"

#include "Nation.h"
#include "Zone.h"
#include "MORPG/Character/Character.h"
#include "MORPG/Quest.h"

#include "TextureManager.h"

#include "ModelManager.h"
#include "Model.h"
#include "Mesh.h"

World world;

World::World()
{
	empty = true;
}

/// Deletes all contents in this world. Makes it ready for loading again.
void World::Delete()
{
	nations.ClearAndDelete();
	zones.ClearAndDelete();
	characters.ClearAndDelete();
	quests.ClearAndDelete();
	empty = true;
}


// x = major, y = minor, z = debug fix number
Vector3i currentVersion(0,0,0);

/// Saves this world to target file. Will save all zones, characters and quests to the same file.
bool World::WriteTo(std::fstream & file)
{
	currentVersion.WriteTo(file);
	name.WriteTo(file);
	return true;
}
/// Loads from target file. Will load all zones, characters and quests from the same file.
bool World::ReadFrom(std::fstream & file)
{
	Vector3i version;
	version.ReadFrom(file);
	/// No other version allowed, for now.
	if (version != currentVersion)
		return false;
	name.ReadFrom(file);
	empty = false;
	return true;
}

/// Based on zones.
void World::GeneratePreviewTexture()
{
	if (!texture)
		texture = TexMan.NewDynamic();
	// Resize it.
	texture->Resize(size);

	texture->SetColor(Vector4f(0.2f,0.2f,0.2f,1));

	// Paint pixels accordingly.
	for (int i = 0; i < zones.Size(); ++i)
	{
		Zone * zone = zones[i];
		Vector4f color = zone->GetColor();
		texture->SetPixel(zone->position, color);
	}
}
/// Based on zones.
void World::GenerateWorldModel()
{
	if (!model)
		model = ModelMan.NewDynamic();

	// Create the mesh for it.
	Mesh * mesh = model->mesh;
	if (!mesh)
	{
		mesh = new Mesh();
		model->mesh = mesh;
	}
	int tiles = size.x * size.y;
	// If the amount of vertices is not the same as the amount suggested by the amount of zones...
	if (mesh->vertices.Size() != tiles)
	{
		// Then delete the existing data.
		mesh->Delete();

		/// Create a regular plane.
		Vector3f topLeft(-1,0,1),
			bottomLeft(-1,0,-1),
			bottomRight(1,0,-1), 
			topRight(1,0,1);
		/// o-o
		mesh->AddPlane(topLeft, bottomLeft, bottomRight, topRight);

		// Subdivide it until it reaches wanted size.

	}
	

}


