/// Emil Hedemalm
/// 2014-08-10
/// A physics mesh based on a regular mesh.


#include "PhysicsMesh.h"
#include "Physics/Collision/CollisionShapeOctree.h"
#include "Mesh/Mesh.h"

#include "PhysicsLib/Shapes/Ray.h"
#include "PhysicsLib/Shapes/Ngon.h"
#include "PhysicsLib/Shapes/Quad.h"

#include "PhysicsLib/Shapes/AABB.h"

#include "File/LogFile.h"

PhysicsMesh::PhysicsMesh(){
	collisionShapeOctree = NULL;
};

PhysicsMesh::~PhysicsMesh(){
	// Deallocate all dynamically allocated members
	triangles.ClearAndDelete();
	quads.ClearAndDelete();
	ngons.ClearAndDelete();
	if (collisionShapeOctree)
		delete collisionShapeOctree;
	collisionShapeOctree = NULL;
};

bool PhysicsMesh::LoadFrom(Mesh * mesh)
{
	// Clear old
	triangles.ClearAndDelete();
	quads.ClearAndDelete();

	name = mesh->name;
	source = mesh->source; 
	aabb = *mesh->aabb;

	for (int i = 0; i < mesh->numFaces; ++i)
	{
		// Just copy shit from it
		MeshFace * faces = &mesh->faces[i];
		assert((faces->numVertices <= 4 || faces->numVertices >= 3) && "Bad vertices count in faces");

		int vi0 = faces->vertices[0],
			vi1 = faces->vertices[1],
			vi2 = faces->vertices[2];

		assert(vi0 < mesh->vertices.Size() && 
			vi0 >= 0);

		Vector3f p1 = mesh->vertices[vi0],
			p2 = mesh->vertices[vi1],
			p3 = mesh->vertices[vi2];

		if (faces->numVertices == 4){
			Vector3f p4 = mesh->vertices[faces->vertices[3]];
			Quad * quad = new Quad();
			quad->Set4Points(p1, p2, p3, p4);
			quads.Add(quad);
		}
		else if (faces->numVertices == 3){
			Triangle * tri = new Triangle();
			tri->Set3Points(p1, p2, p3);
			triangles.Add(tri);
		}
	}
	if (quads.Size() == 0 && triangles.Size() == 0)
		assert(false && "Unable to load physicsmesh from mesh source!");
	else
		std::cout<<"\nCreated physics mesh for \""<<source<<"\": ";
	if (triangles.Size())
		std::cout<<"\n- "<<triangles.Size()<<" triangles";
	if (quads.Size())
		std::cout<<"\n- "<<quads.Size()<<" quads";

	// Re-generate it.
	GenerateCollisionShapeOctree();
	return true;
}

/// Performs a raycast considering target ray and the transform of this physics mesh.
List<Intersection> PhysicsMesh::Raycast(Ray & ray, Matrix4f & transform)
{
	List<Intersection> intersections;
	// o-o
	for (int i = 0; i < triangles.Size(); ++i)
	{
		Triangle triangle = *triangles[i];	
		triangle.Transform(transform);
		// Do intersection test
		float distance;
		if (ray.Intersect(triangle, &distance))
		{
			Intersection newI;
			newI.distance = distance;
			intersections.Add(newI);
		}
	}
	return intersections;
}

/// Generates a collission shape octree that can be used in the local-coordinate system or multiplied by matrices to be used globally.
void PhysicsMesh::GenerateCollisionShapeOctree()
{
	if (collisionShapeOctree == 0)
		collisionShapeOctree = new CollisionShapeOctree();
	else 
	{
		collisionShapeOctree->ClearAll();
	}
	/// Fetch size of object and extend the size of the octree by 10% just to make sure that it works later on.
	Vector3f size = aabb.scale;
	Vector3f min = aabb.min,
		max = aabb.max;
	collisionShapeOctree->SetBoundaries(
		min[0] - size[0] * 0.1f + aabb.position.x - 1.f,
		max[0] + size[0] * 0.1f + aabb.position.x + 1.f,
		max[1] + size[1] * 0.1f + aabb.position.y + 1.f,
		min[1] - size[1] * 0.1f + aabb.position.y - 1.f,
		max[2] + size[2] * 0.1f + aabb.position.z + 1.f,
		min[2] - size[2] * 0.1f + aabb.position.z - 1.f);

	// Adding triangles..
	int skipped = 0;
	for (int i = 0; i < triangles.Size(); ++i)
	{
		Triangle * triangle = triangles[i];
		if (triangle->normal.MaxPart() == 0)
		{
			++skipped;
			continue;
		}
		assert(triangle->normal.MaxPart());
		collisionShapeOctree->AddTriangle(triangles[i]);
//		collisionShapeOctree->PrintContents();
	}
	if (skipped)
		LogPhysics("\n"+String(skipped)+" triangles skipped while generating colision octree for mesh: "+source, INFO);
	
//	collisionShapeOctree->PrintContents();
	
	/// Optimize it! <- Does what?
	collisionShapeOctree->Optimize();
	std::cout<<"\nOptimizing collission shape octree by removing unused child cells.";
//	collisionShapeOctree->PrintContents();
	std::cout<<"\nCollision shape octree generated for mesh: "<<name;
}
