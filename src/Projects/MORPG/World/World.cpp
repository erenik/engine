/// Emil Hedemalm
/// 2014-07-27
/// Class encompassing an entire world.

#include "World.h"

#include "Nation.h"
#include "Zone.h"
#include "MORPG/Character/Character.h"
#include "MORPG/Quest.h"

#include "TextureManager.h"

#include "Model/ModelManager.h"
#include "Model/Model.h"

#include "Mesh/Mesh.h"
#include "Mesh/EMesh.h"
#include "Mesh/EVertex.h"

#include "Graphics/GraphicsManager.h"

World world;
EMesh worldEMesh;

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
Texture * World::GeneratePreviewTexture()
{
	if (!texture)
		texture = TexMan.NewDynamic();

	texture->name = "World Preview texture";
	texture->source = "World::GeneratePreviewTexture";
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
	return texture;
}
/// Based on zones.
Model * World::GenerateWorldModel()
{
	// Pause rendering while doing this...
	Graphics.PauseRendering();

	if (!model)
		model = ModelMan.NewDynamic();

	// Create the mesh for it.
	if (!model->mesh)
	{
		Mesh * mesh = new Mesh();
		model->mesh = mesh;
	}
	int tiles = size.x * size.y;
	// If the amount of vertices is not the same as the amount suggested by the amount of zones...
	if (worldEMesh.vertices.Size() != tiles)
	{
		// Then delete the existing data.
		worldEMesh.Delete();

		/// Create a regular plane.
		Vector3f topLeft(-1,0,1),
			bottomLeft(-1,0,-1),
			bottomRight(1,0,-1), 
			topRight(1,0,1);

		// Add grid of wanted size o-o
		// Since the grid "size" is actually the amount of faces, we will ahve to adjust it so that we instead get 1 vertex per "size"
		Vector2i gridSizeWanted = size - Vector2i(1,1);
		/// Just take -1 on both and we should get the right amount of vertices! :)
		worldEMesh.AddGrid(topLeft, bottomLeft, bottomRight, topRight, size);
	}
	
//	zoneMatrix.PrintContents();
	// Fetch the matrix of vertices created with te grid.
	Matrix<EVertex*> & vertices = worldEMesh.vertexMatrix;

	/// Manipulate them depending on what the tiles were randomized to become!
	for (int x = 0; x < size.x; ++x)
	{
		for (int y = 0; y < size.y; ++y)
		{
			Zone * zone = zoneMatrix[x][y];
			float elevation = 0.f;
			if (!zone->IsWater())
				elevation += 1.f;
			if (zone->IsMountain())
				elevation += 2.f;
			// Just set y.
			EVertex * vertex = vertices[x][y];
			vertex->y = elevation;
		}
	}

	/// Load the new data from the editable mesh into the renderable/optimized one!
	model->mesh->LoadDataFrom(&worldEMesh);
	model->RegenerateTriangulizedMesh();

	Graphics.ResumeRendering();
	return model;
}



Zone * World::GetZoneByName(String name)
{
	for (int i = 0; i < zones.Size(); ++i)
	{
		Zone * zone = zones[i];
		if (zone->name == name)
			return zone;
	}
	return NULL;
}

Zone * World::GetZoneByPosition(Vector3f pos)
{
	Zone * closest = NULL;
	float closestDist = 1000000.f;
	for (int i = 0; i < zones.Size(); ++i)
	{
		Zone * zone = zones[i];
		float dist = (zone->position - pos).Length();
		if (dist < closestDist)
		{
			closestDist = dist;
			closest = zone;
		}
	}
	return closest;
}

/// Like a navmesh..
void World::ConnectZonesByDistance(float minDist)
{
	Zone * zone1, * zone2;
	for (int i = 0; i < zones.Size(); ++i)
	{
		zone1 = zones[i];
		for (int j = i + 1; j < zones.Size(); ++j)
		{
			zone2 = zones[j];
			float dist = (zone1->position - zone2->position).Length();
			if (dist < minDist)
			{
				zone1->neighbours.Add(zone2);
				zone2->neighbours.Add(zone1);
			}
		}
	}
}
