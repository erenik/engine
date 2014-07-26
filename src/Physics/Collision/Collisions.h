// Emil Hedemalm
// 2013-03-19

/// A batch of functions for calculating collissions, whether it be for physics or otherwise!

#ifndef COLLISSIONS_H
#define COLLISSIONS_H

#include "Collision.h"

class OrientedBoundingBox;

/// For them OBBs.
bool OBBOBBCollision(OrientedBoundingBox * obb1, OrientedBoundingBox * obb2, Collision &data);

/// For checking between static shapes (most raw data types, pretty much)
bool TriangleSphereCollision(Triangle * triangle, Sphere * sphere, Collision &data);

/// For checking between static shapes and entities.
bool TriangleSphereCollision(Triangle * triangle, Entity * sphere, Collision &data);
bool QuadSphereCollision(Quad * quad, Entity * sphere, Collision &data);

/// Geometrical collission tests.
bool EdgeFaceCollision(Edge * edge, Face * face, Collision &data);

/// For checking between entities. Transformation matrices are taken into account.
bool SphereSphereCollision(Entity * one, Entity * two, Collision &data);
bool TriangleSphereCollision(Entity * triangle, Entity * sphere, Collision &data);
bool QuadSphereCollision(Entity * quad, Entity * sphere, Collision &data);
bool PlaneSphereCollision(Entity * plane, Entity * sphere, Collision &data);
bool MeshSphereCollision(Entity * meshEntity, Entity * sphereEntity, Collision &data);
bool CubeSphereCollision(Entity * cubeEntity, Entity * sphereEntity, Collision &data);

/// Resolves the active collission between the two entities using the given data. Returns false if they are no longer colliding after resolution.
bool ResolveCollision(Collision &data);



#endif
