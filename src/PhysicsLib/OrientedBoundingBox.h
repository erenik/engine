/// Emil Hedemalm
/// 2013-09-19
/// An oriented bounding bux, for le physicsz!

#ifndef ORIENTEDBOUNDINGBOX_H
#define ORIENTEDBOUNDINGBOX_H

#include "MathLib.h"
#include "PhysicsLib.h"
#include "Model/Geometry.h"
#include "List/List.h"

struct Collision;
class Entity;
struct Edge;
struct Face;

class OrientedBoundingBox {
public:
    OrientedBoundingBox();
    virtual ~OrientedBoundingBox();

    /// Recalculate it using the entity's base model's AABB and current matrix.
    void Recalculate(Entity * entity);
    /// Render with glVertex vertexou
    void Render();

	/// Checks whether the target sphere is within this OBB or not. Might not factor in radius!
	bool IsInside(Sphere & sphere);
	bool Collide(Sphere & sphere, Collision & collissionData);

    /// Lokal Y+, X+ and Z- axes
    Vector3f localUp, localRight, localForward;
    Vector3f position;
    /// Size in local X, Y and Z.
    Vector3f localSize;
    Vector3f localHalfSize;

    static const int CORNERS = 8;
    static const int TRIANGLES = 12;
	static const int EDGES = 12;
	static const int FACES = 6;

    Vector3f corners[CORNERS];
    Triangle triangles[TRIANGLES];

	List<Vertex*> vertexList;
	List<Edge*> edgeList;
	List<Face*> faceList;

protected:

private:

};

#endif // ORIENTEDBOUNDINGBOX_H
