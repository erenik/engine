// Emil Hedemalm
// 2013-07-22

#include "Collisions.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"
#include "PhysicsLib/Shapes/Cube.h"
#include "PhysicsLib/Shapes/Frustum.h"

bool CubeSphereCollision(Entity* cubeEntity, Entity* sphereEntity, Collision &data)
{
	assert(cubeEntity->physics->shapeType == ShapeType::CUBE &&
		sphereEntity->physics->shapeType == ShapeType::SPHERE);


	/// Inspired by the frustum codes o.o;

	Plane frustumPlane[6];

	Vector3f hitherTopLeft(-1,1,1), 
		hitherTopRight(1,1,1), 
		hitherBottomLeft(-1,-1,1), 
		hitherBottomRight(1,-1,1),
		fartherTopLeft(-1,1,-1), 
		fartherTopRight(1,1,-1), 
		fartherBottomLeft(-1,-1,-1), 
		fartherBottomRight(1,-1,-1);
	
	hitherTopLeft *= cubeEntity->transformationMatrix;
	hitherTopRight *= cubeEntity->transformationMatrix;
	hitherBottomLeft *= cubeEntity->transformationMatrix;
	hitherBottomRight *= cubeEntity->transformationMatrix;
	fartherTopLeft *= cubeEntity->transformationMatrix;
	fartherTopRight *= cubeEntity->transformationMatrix;
	fartherBottomLeft *= cubeEntity->transformationMatrix;
	fartherBottomRight *= cubeEntity->transformationMatrix;

	
	/// Save this data for later rendering if it isn't created already, hjaow.
	if (cubeEntity->physics->shape == NULL){
		Cube * cube = new Cube();
		cubeEntity->physics->shape = (void*)cube;
	}
	Cube * c = (Cube*)cubeEntity->physics->shape;
	c->hitherTopLeft = hitherTopLeft;
	c->hitherTopRight = hitherTopRight;
	c->hitherBottomLeft = hitherBottomLeft;
	c->hitherBottomRight = hitherBottomRight;
	c->fartherTopLeft = fartherTopLeft;
	c->fartherTopRight = fartherTopRight;
	c->fartherBottomLeft = fartherBottomLeft;
	c->fartherBottomRight = fartherBottomRight;


	// Create the six planes by giving them three points each, in counter-clockwise order
	// are given in counter clockwise order
	frustumPlane[LEFT_PLANE].Set3Points(hitherTopLeft, hitherBottomLeft, fartherBottomLeft);
	frustumPlane[RIGHT_PLANE].Set3Points(hitherTopRight, fartherTopRight, fartherBottomRight);
	frustumPlane[BOTTOM_PLANE].Set3Points(fartherBottomRight, fartherBottomLeft, hitherBottomLeft);
	frustumPlane[TOP_PLANE].Set3Points(fartherTopRight, hitherTopRight, hitherTopLeft);
	frustumPlane[NEAR_PLANE].Set3Points(hitherBottomLeft, hitherTopLeft, hitherTopRight);
	frustumPlane[FAR_PLANE].Set3Points(fartherBottomLeft, fartherBottomRight, fartherTopRight);	

	float distance;
	int result = Loc::INSIDE;

	for(int i=0; i < 6; i++) 
	{
		distance = frustumPlane[i].Distance(sphereEntity->worldPosition);
		if (distance < -sphereEntity->physics->physicalRadius)
		{
			return false;
		//	return Loc::OUTSIDE;
		}
		else if (distance < sphereEntity->physics->physicalRadius)
			result =  Loc::INTERSECT;
	}
//	std::cout<<"\nCube-Sphere collission!";

	data.collisionNormal = (cubeEntity->worldPosition - sphereEntity->worldPosition).NormalizedCopy();
	data.distanceIntoEachOther = distance;
	data.results |= DISTANCE_INTO;
	return true;
}

