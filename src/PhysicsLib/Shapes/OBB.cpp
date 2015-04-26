/// Emil Hedemalm
/// 2013-09-19

#include "OBB.h"
#include "AABB.h"
#include "Entity/Entity.h"
#include "Graphics/OpenGL.h"
#include "Model/Model.h"
#include "Physics/Collision/Collision.h"

#include "Sphere.h"

OBB::OBB()
{
    //ctor
}

OBB::~OBB()
{
    //dtor
	vertexList.ClearAndDelete();
	edgeList.ClearAndDelete();
	faceList.ClearAndDelete();
}


/// Checks whether the target sphere is within this OBB or not. Might not factor in radius!
bool OBB::IsInside(Sphere & sphere){
	sphere.position;
	Vector3f obbToSphere = sphere.position - position;
	float dot;

	dot = localForward.DotProduct(obbToSphere);
	if (AbsoluteValue(dot) > localHalfSize[2])
		return false;

	dot = localUp.DotProduct(obbToSphere);
	if (AbsoluteValue(dot) > localHalfSize[1])
		return false;

	dot = localRight.DotProduct(obbToSphere);
	if (AbsoluteValue(dot) > localHalfSize[0])
		return false;

	return true;
}

bool OBB::Collide(Sphere & sphere, Collision & collissionData){
	sphere.position;
	Vector3f obbToSphere = sphere.position - position;
	float vecDotForward, vecDotUp, vecDotRight,
		avdf, avdu, avdr;
	
	Vector3f sign(1,1,1);

	vecDotForward = localForward.DotProduct(obbToSphere);
	if (vecDotForward < 0)
		sign[2] = -1;
	avdf = AbsoluteValue(vecDotForward);
	if (avdf > localHalfSize[2])
		return false;

	vecDotUp = localUp.DotProduct(obbToSphere);
	if (vecDotUp < 0)
		sign[1] = -1;
	avdu = AbsoluteValue(vecDotUp);
	if (avdu > localHalfSize[1])
		return false;

	vecDotRight = localRight.DotProduct(obbToSphere);
	if (vecDotRight < 0)
		sign[0] = -1;
	avdr = AbsoluteValue(vecDotRight);
	if (avdr > localHalfSize[0])
		return false;
	
#define FORWARD	1
#define UP		2
#define RIGHT	3
	int closest = 0;
	Vector3f normal;
	float distanceInto = 0.0f;
	/// Find the face which is most likely to be the normal-/collission-face.
	if (avdf > avdu){
		if (avdf > avdr){
			/// Forward was closest!
			closest = FORWARD;
			normal = localForward * sign[2];
			distanceInto = avdf;
		}
		else {
			/// Right was closest!
			closest = RIGHT;
			normal = localRight * sign[0];
			distanceInto = avdr;
		}
	}
	else {
		if (avdu > avdr){
			/// Up closest
			closest = UP;
			normal = localUp * sign[1];
			distanceInto = avdu;
		}
		else {
			/// Right closest;
			closest = RIGHT;
			normal = localRight * sign[0];
			distanceInto = avdr;
		}
	}
	collissionData.distanceIntoEachOther = distanceInto;
	collissionData.collissionPoint = sphere.position;

	/// If flagged, assert that the preliminary normal somewhat coincides with the final result!
	if (collissionData.results & PRELIMINARY_COLLISSION_NORMAL){
		float dot = collissionData.preliminaryCollisionNormal.DotProduct(normal);
	//	assert(dot > 0.9f);
		if (dot < 0.9f){
			std::cout<<"\nWARNING: Collision normal does not align with the preliminary normal! Bahaaa! ö.ö";	
		}
	}
	collissionData.collisionNormal = normal;
	

	return true;
}


/// Recalculate it using the entity's base model's AABB and current matrix.
void OBB::Recalculate(Entity * entity)
{
	if (!entity->model->mesh->aabb)
		return;
	// Edges are created below, so clear 'em.
	if(edgeList.Size() != 0)
		edgeList.ClearAndDelete();
	if (faceList.Size() != 0)
		faceList.ClearAndDelete();

	assert(entity->model);
    assert(entity->physics);
//    std::cout<<"\nRecalculating OBB for entity "<<entity<<" "<<entity->position;
    const AABB & aabb = entity->model->GetAABB();
    const Vector3f & min = aabb.min, & max = aabb.max;
	localSize = max - min;
	localHalfSize = localSize * 0.5f;

    corners[0] = Vector3f(min.x, min.y, min.z);
    corners[1] = Vector3f(max.x, min.y, min.z);
    corners[2] = Vector3f(max.x, max.y, min.z);
    corners[3] = Vector3f(min.x, max.y, min.z);
    corners[4] = Vector3f(min.x, min.y, max.z);
    corners[5] = Vector3f(max.x, min.y, max.z);
    corners[6] = Vector3f(max.x, max.y, max.z);
    corners[7] = Vector3f(min.x, max.y, max.z);

	localUp = entity->rotationMatrix.Product(Vector3f(0,1,0));
	localForward = entity->rotationMatrix.Product(Vector3f(0,0,-1));
	localRight = entity->rotationMatrix.Product(Vector3f(1,0,0));

	Vector3f average;
    for (int i = 0; i < CORNERS; ++i){
   //     std::cout<<"\nCorner "<<i<<": "<<corners[i];
        /// Transform it.
        corners[i] = entity->transformationMatrix.Product(corners[i]);
   //     std::cout<<" -> "<<corners[i];
		average += corners[i];
    }

	position = average / CORNERS;


    /// Create ze triangles. This since the normals will need recalculating anyway.
#define c(i) corners[i]
    /// Hither
    triangles[0].Set3Points(c(0), c(1), c(2));
    triangles[1].Set3Points(c(1), c(2), c(3));
    /// Farther
    triangles[2].Set3Points(c(4), c(7), c(6));
    triangles[3].Set3Points(c(7), c(6), c(4));
    /// Left
    triangles[4].Set3Points(c(4), c(0), c(3));
    triangles[5].Set3Points(c(0), c(3), c(7));
    /// Right
    triangles[6].Set3Points(c(1), c(5), c(6));
    triangles[7].Set3Points(c(5), c(6), c(2));
    /// Top
    triangles[8].Set3Points(c(3), c(2), c(6));
    triangles[9].Set3Points(c(2), c(6), c(7));
    /// Bottom
    triangles[10].Set3Points(c(0), c(4), c(5));
    triangles[11].Set3Points(c(4), c(5), c(1));

	/// If we're lacking the list-contents, create them now.
	if (vertexList.Size() == 0){
		for (int i = 0; i < CORNERS; ++i)
			vertexList.Add(new Vertex(corners[i]));
		
		edgeList.Add(new Edge(vertexList[0], vertexList[1]));
		edgeList.Add(new Edge(vertexList[1], vertexList[2]));
		edgeList.Add(new Edge(vertexList[2], vertexList[3]));
		edgeList.Add(new Edge(vertexList[3], vertexList[0]));

		edgeList.Add(new Edge(vertexList[4], vertexList[7]));
		edgeList.Add(new Edge(vertexList[7], vertexList[6]));
		edgeList.Add(new Edge(vertexList[6], vertexList[5]));
		edgeList.Add(new Edge(vertexList[5], vertexList[4]));
		
		edgeList.Add(new Edge(vertexList[0], vertexList[4]));
		edgeList.Add(new Edge(vertexList[1], vertexList[5]));
		edgeList.Add(new Edge(vertexList[2], vertexList[6]));
		edgeList.Add(new Edge(vertexList[3], vertexList[7]));

#define FACE(v0,v1,v2,v3);	faceList.Add(new Face(vertexList[v0], vertexList[v1], vertexList[v2], vertexList[v3]));

		FACE(0,1,2,3);  // Hither, 0
		FACE(4,7,6,5);	// Farther, 1

		FACE(4,0,3,7); // Left, 2
		FACE(1,5,6,2); // Right, 3

		FACE(3,2,6,7); // Top, 4
		FACE(0,4,5,1); // Bottom, 5

		/// Recalculate the normals of the faces.
		for (int i = 0; i < faceList.Size(); ++i){
			faceList[i]->RecalculateNormal();
		}


		/// Bind edges and faces..
		int verticesInCommon;
		for (int i = 0; i < edgeList.Size(); ++i)
		{
			Edge * e = edgeList[i];
			for (int j = 0; j < faceList.Size(); ++j)
			{
				int verticesInCommon = 0;
				Face * f = faceList[j];
				for (int k = 0; k < f->vertexList.Size(); ++k)
				{
					Vertex * v = f->vertexList[k];
					if (v == e->start || 
						v == e->stop)
						++verticesInCommon;
				}
				/// Add the edge to the face and the face to the edge if they coincide.
				if (verticesInCommon >= 2)
				{
					e->faceList.Add(f);
					f->edgeList.Add(e);
				}
			}
			/// There should be at least two faces for each edge..
			assert(e->faceList.Size() >= 2);
			Vector3f edgeNormal = (e->faceList[0]->normal + e->faceList[1]->normal).NormalizedCopy();
	//		std::cout<<"\nEdge normal: "<<edgeNormal;
		}
	}

	/// Update the vertices' positions of the vertexList.
	for (int i = 0; i < vertexList.Size(); ++i){
		vertexList[i]->position = corners[i];
	}
	/// Recalculate the normals of the faces.
	for (int i = 0; i < faceList.Size(); ++i)
	{
		faceList[i]->RecalculateNormal();
		faceList[i]->RecalculateTriangles();
	}

}


void OBB::Render(){
    glBegin(GL_LINE_STRIP);
#define RENDER(p);  glVertex3f(p[0],p[1],p[2]);
    glColor3f(1,0,0);
    RENDER(corners[0]);

    glColor3f(1,0.5,0);
    RENDER(corners[1]);

    glColor3f(1,1,0);
    RENDER(corners[3]);

    glColor3f(0,1,0);
    RENDER(corners[2]);

    glColor3f(1,0,0);
    RENDER(corners[0]);

    glColor3f(1,0,1);
    RENDER(corners[4]);

    glColor3f(0.5,0,1);
    RENDER(corners[5]);

    glColor3f(0,0,1);
    RENDER(corners[7]);

    glColor3f(0,0.5f,1);
    RENDER(corners[6]);

    glColor3f(0,1,0);
    RENDER(corners[2]);

    glEnd();
}
