// Emil Hedemalm
// 2013-03-19

/// A batch of functions for calculating collissions, whether it be for physics or otherwise!

#ifndef COLLISSIONS_H
#define COLLISSIONS_H

#include "Collision.h"

class Triangle;
class Edge;
class Face;
class Sphere;
class Quad;
class OBB;

/** Tests if a collission should occur between the two objects.
	If so, it will save the collission data into the data parameter and return true.
	If no collission should occur, it will return false.
*/
bool TestCollision(Entity* one, Entity* two, List<Collision> & collissionList);
// The below functions are used to perform the actual tests once their types have been determined.

/// For them OBBs.
bool OBBOBBCollision(OBB * obb1, OBB * obb2, Collision &data);

/// For checking between static shapes (most raw data types, pretty much)
bool TriangleSphereCollision(Triangle * triangle, Sphere * sphere, Collision &data, bool planesOnly);

/// For checking between static shapes and entities. If planesOnly is true, skips edge and corner/point tests which may be faster but produce other/more bugs.
bool TriangleSphereCollision(Triangle * triangle, Entity* sphere, Collision &data, bool planesOnly);
bool QuadSphereCollision(Quad * quad, Entity* sphere, Collision &data, bool planesOnly);
bool AABBSphereCollision(AABB * aabb, Entity* sphere, Collision &data, bool planesOnly);

/// Geometrical collission tests.
bool EdgeFaceCollision(Edge * edge, Face * face, Collision &data);

/// For checking between entities. Transformation matrices are taken into account.
bool SphereSphereCollision(Entity* one, Entity* two, Collision &data);
bool TriangleSphereCollision(Entity* triangle, Entity* sphere, Collision &data, bool planesOnly);
bool QuadSphereCollision(Entity* quad, Entity* sphere, Collision &data, bool planesOnly);
bool PlaneSphereCollision(Entity* plane, Entity* sphere, Collision &data);
bool MeshSphereCollision(Entity* meshEntity, Entity* sphereEntity, Collision &data);
bool CubeSphereCollision(Entity* cubeEntity, Entity* sphereEntity, Collision &data);

/// For checking between a set shape/entity and a bunch of basic shapes (e.g. faces/tris/quads). Supply matrix if the tris or quads should be recalculated.
bool SphereFaceCollision(Entity* sphereEntity, const List<Triangle*> & tris, const List<Quad*> & quads, Collision & data, const Matrix4f * triQuadMatrix = 0, bool planesOnly = false);


/// Resolves the active collission between the two entities using the given data. Returns false if they are no longer colliding after resolution.
bool ResolveCollision(Collision &data);



#endif
