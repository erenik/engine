/// Emil Hedemalm
/// 2016-06-11
/// Generalized collision tester for a sphere (or Sphere-based entity) and a number of triangles or quads.

#include "Collisions.h"
#include "Collision.h"
#include "Model/Geometry.h"
#include "PhysicsLib/Shapes/Quad.h"
#include "Physics/PhysicsProperty.h"
#include "PhysicsLib/PhysicsMesh.h"

/// For checking between a set shape/entity and a bunch of basic shapes (e.g. faces/tris/quads). Supply matrix if the tris or quads should be recalculated.
bool SphereFaceCollision(Entity* sphereEntity, const List<Triangle*> & tris, const List<Quad*> & quads, Collision & data, const Matrix4f * triQuadMatrix, bool planesOnly)
{
	/// Test for all, only evaluate the deepest collission! o.o'
	Collision deepestCollision;
	float deepestCollisionDistance = 0.0f;
	Triangle tri;
	Quad quad;
	/// With per-tri/-quad local transformations.
	if (triQuadMatrix)
	{
		for (int i = 0; i < tris.Size(); ++i)
		{
			tri = *tris[i];
			tri.Transform(*triQuadMatrix);
			if (TriangleSphereCollision(&tri, sphereEntity, data, planesOnly))
			{
				if (abs(data.distanceIntoEachOther) > deepestCollisionDistance)
				{
					deepestCollisionDistance = abs(data.distanceIntoEachOther);
					deepestCollision = data;
					deepestCollision.activeTriangles.Add(tri);
				}
			}
		}
		for (int i = 0; i < quads.Size(); ++i)
		{
			quad = *quads[i];
			quad.Transform(*triQuadMatrix);
			if (QuadSphereCollision(&quad, sphereEntity, data, planesOnly))
			{
				if (abs(data.distanceIntoEachOther) > deepestCollisionDistance)
				{
					deepestCollisionDistance = abs(data.distanceIntoEachOther);
					deepestCollision = data;
				}
			}
		}		
	}
	else { 
		/// No per-tri/-quad transform.
		for (int i = 0; i < tris.Size(); ++i)
		{
			tri = *tris[i];
			if (TriangleSphereCollision(&tri, sphereEntity, data, planesOnly))
			{
				if (abs(data.distanceIntoEachOther) > deepestCollisionDistance)
				{
					deepestCollisionDistance = abs(data.distanceIntoEachOther);
					deepestCollision = data;
					deepestCollision.activeTriangles.Add(tri);
				}
			}
		}
		for (int i = 0; i < quads.Size(); ++i)
		{
			quad = *quads[i];
			if (QuadSphereCollision(&quad, sphereEntity, data, planesOnly))
			{
				if (abs(data.distanceIntoEachOther) > deepestCollisionDistance)
				{
					deepestCollisionDistance = abs(data.distanceIntoEachOther);
					deepestCollision = data;
				}
			}
		}
	}


	if (deepestCollisionDistance > ZERO){
		data = deepestCollision;
		return true;
	}
	return false;
}



