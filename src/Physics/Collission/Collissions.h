// Emil Hedemalm
// 2013-03-19

/// A batch of functions for calculating collissions, whether it be for physics or otherwise!

#ifndef COLLISSIONS_H
#define COLLISSIONS_H

#include "Collission.h"

class OrientedBoundingBox;

/// For them OBBs.
bool OBBOBBCollission(OrientedBoundingBox * obb1, OrientedBoundingBox * obb2, Collission &data);

/// For checking between static shapes (most raw data types, pretty much)
bool TriangleSphereCollission(Triangle * triangle, Sphere * sphere, Collission &data);

/// For checking between static shapes and entities.
bool TriangleSphereCollission(Triangle * triangle, Entity * sphere, Collission &data);
bool QuadSphereCollission(Quad * quad, Entity * sphere, Collission &data);

/// Geometrical collission tests.
bool EdgeFaceCollission(Edge * edge, Face * face, Collission &data);

/// For checking between entities. Transformation matrices are taken into account.
bool SphereSphereCollission(Entity * one, Entity * two, Collission &data);
bool TriangleSphereCollission(Entity * triangle, Entity * sphere, Collission &data);
bool QuadSphereCollission(Entity * quad, Entity * sphere, Collission &data);
bool PlaneSphereCollission(Entity * plane, Entity * sphere, Collission &data);
bool MeshSphereCollission(Entity * meshEntity, Entity * sphereEntity, Collission &data);
bool CubeSphereCollission(Entity * cubeEntity, Entity * sphereEntity, Collission &data);

/// Resolves the active collission between the two entities using the given data. Returns false if they are no longer colliding after resolution.
bool ResolveCollission(Collission &data);



#endif
