// Emil Hedemalm
// 2013-03-17
#include "CollissionShapeOctree.h"
#include "Physics/PhysicsProperty.h"
#include "Physics/PhysicsManager.h"
#include "Graphics/GraphicsManager.h"
#include <GL/glew.h>
#include "GraphicsState.h"
#include "Graphics/Camera/Camera.h"
#include "Collission.h"
extern PhysicsManager physics;

/// Define the static values

/// Basic constructor
CollissionShapeOctree::CollissionShapeOctree(){
	for (int i = 0; i < MAX_CHILD_NODES; ++i){
		child[i] = NULL;
	}

	// Default Entity pointer amounts.
	subdivision = 0;
	parent = NULL;
}

/// Internal constructor that sets the Entity's boundaries and subdivision level
CollissionShapeOctree::CollissionShapeOctree(float i_leftBound, float i_rightBound, float i_topBound, float i_bottomBound, float i_nearBound, float i_farBound, int i_subdivision){
	for (int i = 0; i < MAX_CHILD_NODES; ++i){
		child[i] = NULL;
	}
	// Set boundaries, calculate radius, etc.
	SetBoundaries(i_leftBound, i_rightBound, i_topBound, i_bottomBound, i_nearBound, i_farBound);
	subdivision = i_subdivision;
}

/// Default destructor that deallocates all children
CollissionShapeOctree::~CollissionShapeOctree(){
	if (child[0]){
		for (int i = 0; i < MAX_CHILDREN; ++i){
			delete child[i];
			child[i] = NULL;
		}
	}
}

/// Sets boundaries of the top-level node.
void CollissionShapeOctree::SetBoundaries(float leftBound, float rightBound, float topBound, float bottomBound, float i_nearBound, float i_farBound){
	left = leftBound;
	right = rightBound;
	top = topBound;
	bottom = bottomBound;
	nearBound = i_nearBound;
	farBound = i_farBound;
	center = Vector3f(
		(right + left) / 2.0f,
		(top + bottom) / 2.0f,
		(nearBound + farBound) / 2.0f
	);
	/// Calculate radial radius straight away too.
	float width = (left - right) / 2.0f;
	float height = (top - bottom) / 2.0f;
	float depth = (nearBound - farBound) / 2.0f;

	radius = sqrt((width * width + height * height + depth * depth));
}


/** Recursive subdivision function that subdivides the nodes down the tree specified amount of levels.
	If MAX_SUBDIVISION is passed an exception will be thrown.
*/
void CollissionShapeOctree::subdivide(int levels){
	// Decrement levels
	levels--;
	// Check that the maximum subdivision level hasn't been reached.
	int childSubdivisionLvl = subdivision + 1;
	if (childSubdivisionLvl > MAX_SUBDIVISION)
		throw 4;
	// Check if the children aren't already allocated.
	if (!child[0]){
		// Allocate if needed
		child[HITHER_LOWER_LEFT] = new CollissionShapeOctree(left, center.x, center.y, bottom, nearBound, center.z, childSubdivisionLvl);
		child[HITHER_LOWER_RIGHT] = new CollissionShapeOctree(center.x, right, center.y, bottom, nearBound, center.z, childSubdivisionLvl);
		child[HITHER_UPPER_LEFT] = new CollissionShapeOctree(left, center.x, top, center.y, nearBound, center.z, childSubdivisionLvl);
		child[HITHER_UPPER_RIGHT] = new CollissionShapeOctree(center.x, right, top, center.y, nearBound, center.z, childSubdivisionLvl);
		child[FARTHER_LOWER_LEFT] = new CollissionShapeOctree(left, center.x, center.y, bottom, center.z, farBound, childSubdivisionLvl);
		child[FARTHER_LOWER_RIGHT] = new CollissionShapeOctree(center.x, right, center.y, bottom, center.z, farBound, childSubdivisionLvl);
		child[FARTHER_UPPER_LEFT] = new CollissionShapeOctree(left, center.x, top, center.y, center.z, farBound, childSubdivisionLvl);
		child[FARTHER_UPPER_RIGHT] = new CollissionShapeOctree(center.x, right, top, center.y, center.z, farBound, childSubdivisionLvl);

		// UPPER_LEFT, UPPER_RIGHT, LOWER_LEFT, LOWER_RIGHT, FARTHER_DOWN, FARTHER_UP, HITHER_DOWN, HITHER_UP,
		child[UPPER_LEFT] = new CollissionShapeOctree(left, center.x, top, center.y, (center.z + nearBound)*0.5f, (center.z + farBound)*0.5f, childSubdivisionLvl);
		child[UPPER_RIGHT] = new CollissionShapeOctree(center.x, right, top, center.y, (center.z + nearBound)*0.5f, (center.z + farBound)*0.5f, childSubdivisionLvl);
		child[LOWER_LEFT] = new CollissionShapeOctree(left, center.x, center.y, bottom, (center.z + nearBound)*0.5f, (center.z + farBound)*0.5f, childSubdivisionLvl);
		child[LOWER_RIGHT] = new CollissionShapeOctree(center.x, right, center.y, bottom, (center.z + nearBound)*0.5f, (center.z + farBound)*0.5f, childSubdivisionLvl);
		child[FARTHER_DOWN] = new CollissionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, center.y, bottom, center.z, farBound, childSubdivisionLvl);
		child[FARTHER_UP] = new CollissionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, top, center.y, center.z, farBound, childSubdivisionLvl);
		child[HITHER_DOWN] = new CollissionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, center.y, bottom, nearBound, center.z, childSubdivisionLvl);
		child[HITHER_UP] = new CollissionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, top, center.y, nearBound, center.z, childSubdivisionLvl);

		// Center
		child[CENTER] = new CollissionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, (center.y+top)*0.5f, (center.y+bottom)*0.5f, (center.z+nearBound)*0.5, (center.z+farBound)*0.5, childSubdivisionLvl);

		// + Center-branches
		//HITHER, FARTHER, LEFTER, RIGHTER, UPPER, LOWER,
		child[HITHER] = new CollissionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5, (center.y+top)*0.5f, (center.y+bottom)*0.5f, nearBound, center.z, childSubdivisionLvl);
		child[FARTHER] = new CollissionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5, (center.y+top)*0.5f, (center.y+bottom)*0.5f, center.z, farBound, childSubdivisionLvl);
		child[LEFTER] = new CollissionShapeOctree(left, center.x, (center.y+top)*0.5f, (center.y+bottom)*0.5f, (center.z+nearBound)*0.5f, (center.z+farBound)*0.5f, childSubdivisionLvl);
		child[RIGHTER] = new CollissionShapeOctree(center.x, right, (center.y+top)*0.5f, (center.y+bottom)*0.5f, (center.z+nearBound)*0.5f, (center.z+farBound)*0.5f, childSubdivisionLvl);
		child[UPPER] = new CollissionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, top, center.y, (center.z+nearBound)*0.5f, (center.z+farBound)*0.5f, childSubdivisionLvl);
		child[LOWER] = new CollissionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, center.y, bottom, (center.z+nearBound)*0.5f, (center.z+farBound)*0.5f, childSubdivisionLvl);

		// + Huggers(apartments, whatever)
		// FATHER_LEFT, FARTHER_RIGHT, HITHER_LEFT, HITHER_RIGHT,
		child[FATHER_LEFT] = new CollissionShapeOctree(left, center.x, (center.y+top)*0.5f, (center.y+bottom)*0.5f, center.z, farBound, childSubdivisionLvl);
		child[FARTHER_RIGHT] = new CollissionShapeOctree(center.x, right, (center.y+top)*0.5f, (center.y+bottom)*0.5f, center.z, farBound, childSubdivisionLvl);
		child[HITHER_LEFT] = new CollissionShapeOctree(left, center.x, (center.y+top)*0.5f, (center.y+bottom)*0.5f, nearBound, center.z, childSubdivisionLvl);
		child[HITHER_RIGHT] = new CollissionShapeOctree(center.x, right, (center.y+top)*0.5f, (center.y+bottom)*0.5f, nearBound, center.z, childSubdivisionLvl);
	}
	for (int i = 0; i < MAX_CHILDREN; ++i)
		child[i]->parent = this;
	// Subdivide all children further if levels is still positive.
	if (levels > 0){
		for (int i = 0; i < MAX_CHILDREN; ++i)
			child[i]->subdivide(levels);
	}
}

void CollissionShapeOctree::clearAll(){
	triangles.Clear();
	if (child[0])
		for (int i = 0; i < MAX_CHILDREN; ++i)
			child[i]->clearAll();
}


/** Adds a scenegraph Entity to this vfcOctree Entity unless max nodes has been reached.
	If MAX_INITIAL_NODES_BEFORE_SUBDIVISION is reached, subdivision hasn't yet been done and MAX_SUBDIVISION hasn't been reached, subdivision occurs.
*/
bool CollissionShapeOctree::AddTriangle(Triangle * tri){
	// Check that it isn't already added!
	bool exists = triangles.Exists(tri);
	if (exists){
		std::cout<<"\nAdding Entity to CollissionShapeOctree while it already exists!";
		assert("Adding Entity to CollissionShapeOctree while it already exists!" && false);
		return false;
	}
	// If we have children, check if it fits in any of them.
	if (child[0]){
		int result = OUTSIDE;
		for (int i = 0; i < MAX_CHILDREN; ++i){
			result = child[i]->IsTriangleInside(tri);
			switch(result){
					// If the Entity is outside, just check next child-node~
				case OUTSIDE:
					continue; 	break;
					// If intersecting, don't do anything. Check all children and handle intersections after this for-loop.
				case INTERSECT: break;
					// If it is inside, continue down the chain and then return from hierrr.
				case INSIDE:
					return child[i]->AddTriangle(tri);
			}
		}
		// If we arrived here, it's either intersecting or something, so add it to our current children since it can't go further down the tree.
		triangles.Add(tri);
		return true;
	} /// End of trying to enter it into any of our children

	// Okay, no spot in children, check if we should subdivide it (if the children aren't already allocated, that is!)
	if (triangles.Size() > MAX_INITIAL_NODES_BEFORE_SUBDIVISION && child[0] == NULL){
		// Subdivide and then try push all our children down the tree further, so they don't get stuck here without reason.
		subdivide();
		List<Triangle*> tempList(triangles);
		triangles.Clear();
		for (int j = 0; j < tempList.Size(); ++j){
			AddTriangle(tempList[j]);
		}
		// Return if we subdivided, just make sure we try to add our new child to the subdivision too.
		return AddTriangle(tri);
	}
	// Alright, children are out, just add it now then.
	triangles.Add(tri);
	return true;
}

/** Polls the existence of target entity with in this node (or any of it's children). */
bool CollissionShapeOctree::Exists(Triangle * tri){
	if (triangles.Exists(tri))
		return true;
	if (child[0]){
		for (int i = 0; i < MAX_CHILDREN; ++i)
			if (child[i]->Exists(tri))
				return true;
	}
	return false;
}

/// Removes the Entity and re-inserts it to it's new location
bool CollissionShapeOctree::RepositionTriangle(Triangle * tri){
	RemoveTriangle(tri);
	AddTriangle(tri);
	return true;
}

// Removes a scenegraph Entity from this vfcOctree. Searches recursively until.
bool CollissionShapeOctree::RemoveTriangle(Triangle * tri){
	if (triangles.Remove(tri)){
		return true;
	}
	// Go through all children. If one of them finds the target Entity, return true without processing any more children.
	if (child[0])
		for (int i = 0; i < MAX_CHILDREN; ++i)
			if (child[i]->RemoveTriangle(tri))
				return true;
	if (this->subdivision == 0){
		assert(this->Exists(tri));
		assert(false && "Unable to remove entity in CollissionShapeOctree::RemoveEntity!");
	}
	return false;
}

/// Returns a count of all registered triangles within the vfcOctree
int CollissionShapeOctree::RegisteredShapes(){
	int total = 0;
	total += triangles.Size();
	if (child [0]){
		for (int i = 0; i < MAX_CHILDREN; ++i){
			total += child[i]->RegisteredShapes();
		}
	}
	return total;
}

/// Prints contents in each cubicle
void CollissionShapeOctree::PrintContents(){
	std::cout<<"\nCollissionShapeOctree: ";
	String spacing;
	for (int i = 0; i < subdivision; ++i){
		spacing += " ";
	}
	std::cout<<spacing;
	std::cout<<"- Triangles: "<<triangles.Size();

	/// Print children if any
	if (child[0]){
		for (int i = 0; i < MAX_CHILD_NODES; ++i){
			child[i]->PrintContents();
		}
	}
}

/// To be used via the GraphicsManager's RenderPhysics function ONLY.
bool CollissionShapeOctree::Render(void * graphicsState){
	// If the camera is inside here, render with hard mode-desu for clarity's sake.
	GraphicsState * graphics = (GraphicsState*) graphicsState;
	Vector3f cam = graphics->camera->Position();
	bool isInside = false, wasInside = false;

	/// Check if inside any children, render only that child if so
	int childIndex = -1;
	// Render children first
	if (child[0]){
		for (int i = 0; i < MAX_CHILD_NODES; ++i){

			if (cam.x < child[i]->right &&
				cam.x > child[i]->left &&
				cam.y < child[i]->top &&
				cam.y > child[i]->bottom &&
				cam.z > child[i]->farBound &&
				cam.z < child[i]->nearBound)
				childIndex = i;
			break;
		}
	}
/*
	if (childIndex != -1){
		child[childIndex]->Render(graphicsState);
		return true;
	}
*/
//	if (isInside)
//		glDepthMask(GL_TRUE);


	// Render children first
	if (child[0]){
		for (int i = 0; i < MAX_CHILD_NODES; ++i){
			/// Render, and if true (meaning it was inside), don't render ourselves or any more stuff
			if (child[i]->Render(graphicsState))
				wasInside = true;
		}
	}
	/// Skip parents if we're inside a sub-element
	if (wasInside && subdivision == 0)
		return true;

	/// Skip cells without any triangles in them at all
	if (triangles.Size() > 0){

		// Draw all triangles within
	//	glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_LINE);
	//	glDisable(GL_DEPTH_TEST);
		glColor4f(AbsoluteValue(left / right), AbsoluteValue(top / bottom), AbsoluteValue(nearBound / farBound), 0.2f);
		glBegin(GL_TRIANGLES);
		for (int q = 0; q < triangles.Size(); ++q){
			Triangle * tri = triangles[q];
			glVertex3f(tri->point1.x, tri->point1.y, tri->point1.z);
			glVertex3f(tri->point2.x, tri->point2.y, tri->point2.z);
			glVertex3f(tri->point3.x, tri->point3.y, tri->point3.z);
		}
		glEnd();



		/*
		// Draw the octree sections too!
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_DEPTH_TEST);
		glColor4f(abs(left / right), abs(top / bottom), abs(nearBound / farBound), 0.05f * (subdivision+1));
		glBegin(GL_QUADS);
		/// Hither side
			glVertex3f(left, top, nearBound);
			glVertex3f(right, top, nearBound);
			glVertex3f(right, bottom, nearBound);
			glVertex3f(left, bottom, nearBound);
		/// Left side
			glVertex3f(left, top, farBound);
			glVertex3f(left, top, nearBound);
			glVertex3f(left, bottom, nearBound);
			glVertex3f(left, bottom, farBound);
		/// Top side
			glVertex3f(left, top, farBound);
			glVertex3f(right, top, farBound);
			glVertex3f(right, top, nearBound);
			glVertex3f(left, top, nearBound);
		glEnd();
		*/
	}
	if (isInside)
		return true;
}

static int collissionsTested = 0;

/// Compare each dynamic entity with every other entity in it's current vfcOctree node,
/// all nodes below it as well the direct parents above it.
int CollissionShapeOctree::FindCollissions(Entity * targetEntity, List<Collission> & collissionList, int entrySubdivisionLevel){
	Matrix4f identityMatrix;
	return FindCollissions(targetEntity, collissionList, identityMatrix);
}

/** Searches for collissions with specified entity.
	If entry subdivision level is not specified the initial call will set it automatically (used for recursion limits)
	Returns amount of collissions tested.
	- localTransform is applied to all relevant triangle upon testing if provided.
*/
int CollissionShapeOctree::FindCollissions(Entity * targetEntity, List<Collission> & collissionList, Matrix4f & localTransform, int entrySubdivisionLevel/* = -1*/)
{
	if (entrySubdivisionLevel == -1)
		entrySubdivisionLevel = this->subdivision;

	int collissionsTested = 0;
	/// First it's current node:
	for (int i = 0; i < triangles.Size(); ++i){

		Triangle * trianglePointer = triangles[i];
		assert(trianglePointer && "Nullentity in CollissionShapeOctree::FindCollissions");
		Triangle tri = *trianglePointer;
		tri.Transform(localTransform);
		if (Graphics.renderPhysics && Graphics.renderCollissionTriangles)
			Physics.activeTriangles.Add(tri);
		Collission col;
		// Check physics type. If Entity has physics type mesh then we should try and optimize it in some other way.
		if (targetEntity->physics->type == ShapeType::MESH){
			std::cout<<"\nWARNING: Entity of type mesh trying to collide with a single Triangle! Is this the intent?";
			assert(targetEntity->physics->type != ShapeType::MESH && "CollissionShapeOctree::FindCollissions mesh-triangle collission where unwanted!");
			return collissionsTested;
		}
		// Entity shape-type!
		switch(targetEntity->physics->physicsShape){
			case ShapeType::SPHERE: {
				if (tri.point1.x == 38.05821)
					i = i;
				Sphere * sphere = (Sphere*)targetEntity->physics->shape;
				sphere->position = targetEntity->positionVector;
				sphere->radius = targetEntity->physics->physicalRadius;
				if (TriangleSphereCollission(&tri, sphere, col)){
				//	col.distanceIntoEachOther
				//	col.one = targetEntity;
				//	col.two =
					collissionList.Add(col);
					Physics.activeTriangles.Add(tri);
				}
				break;
			}
			default:
				std::cout<<"\nUnsupported shape-type for specified mesh when checking collissions vs. Triangles in CollissionShapeOctree::FindCollissions!";
				assert(false && "Unsupported shape-type for specified mesh when checking collissions vs. Triangles in CollissionShapeOctree::FindCollissions!");
				break;
		}
		++collissionsTested;
	}

	/// If entry node, begin all recursion.
	if (this->subdivision == entrySubdivisionLevel){
		/// Then all children
		if (child[0] && this->subdivision >= entrySubdivisionLevel){
			for (int i = 0; i < MAX_CHILDREN; ++i){
				/// Do the actual culling with this continue-statement, yo!
				if (child[i]->IsEntityInside(targetEntity, localTransform) == OUTSIDE)
					continue;
				collissionsTested += child[i]->FindCollissions(targetEntity, collissionList, localTransform, entrySubdivisionLevel);
			}
		}
		/// And at last all parents
		if (parent && this->subdivision <= entrySubdivisionLevel)
			collissionsTested += parent->FindCollissions(targetEntity, collissionList, localTransform, entrySubdivisionLevel);
		return collissionsTested;
	}
	/// Process children if subdivision level is higher (further down the tree.)
	else if (this->subdivision > entrySubdivisionLevel){
		/// Then all children
		if (child[0]){
			for (int i = 0; i < MAX_CHILDREN; ++i){
				/// Do the actual culling with this continue-statement, yo!
				if (child[i]->IsEntityInside(targetEntity, localTransform) == OUTSIDE)
					continue;
				collissionsTested += child[i]->FindCollissions(targetEntity, collissionList, localTransform, entrySubdivisionLevel);
			}
		}
		return collissionsTested;
	}
	/// And process parents only if lesser subdivision
	else if (this->subdivision < entrySubdivisionLevel){
		/// Then go to next parent.
		if (parent && this->subdivision <= entrySubdivisionLevel)
			collissionsTested += parent->FindCollissions(targetEntity, collissionList, localTransform, entrySubdivisionLevel);
		return collissionsTested;
	}
	assert(false && "CollissionShapeOctree::FindCollissions at a point where it shouldn't be!");
	return 0;
}

/// Checks if the target Entity is inside this CollissionShapeOctree Entity, intersecting it or outside.
int CollissionShapeOctree::IsEntityInside(Entity * entity, Matrix4f & localTransform){
	// Make box test insteeeead
//	return INSIDE;

	/// Transform the vectors to the local transform position.
	Vector4f minVec(left, bottom, farBound, 1.0f);
	Vector4f maxVec(right, top, nearBound, 1.0f);
	minVec = localTransform * minVec;
	maxVec = localTransform * maxVec;
	// Check if it's inside.
	if (entity->positionVector.x + entity->radius < maxVec.x &&
		entity->positionVector.x - entity->radius > minVec.x &&
		entity->positionVector.y + entity->radius < maxVec.y &&
		entity->positionVector.y - entity->radius > minVec.y &&
		entity->positionVector.z + entity->radius < maxVec.z &&
		entity->positionVector.z - entity->radius > minVec.z
		)
		return INSIDE;
	// Or intersecting, just compare with inverted radius
	else if (entity->positionVector.x - entity->radius < maxVec.x &&
		entity->positionVector.x + entity->radius > minVec.x &&
		entity->positionVector.y - entity->radius < maxVec.y &&
		entity->positionVector.y + entity->radius > minVec.y &&
		entity->positionVector.z - entity->radius < maxVec.z &&
		entity->positionVector.z + entity->radius > minVec.z
		)
		return INTERSECT;
	/*
	// Check if it's inside.
	if (entity->positionVector.x + entity->radius < right &&
		entity->positionVector.x - entity->radius > left &&
		entity->positionVector.y + entity->radius < top &&
		entity->positionVector.y - entity->radius > bottom &&
		entity->positionVector.z + entity->radius < nearBound &&
		entity->positionVector.z - entity->radius > farBound
	)
		return INSIDE;
	// Or intersecting, just compare with inverted radius
	else if (entity->positionVector.x - entity->radius < right &&
		entity->positionVector.x + entity->radius > left &&
		entity->positionVector.y - entity->radius < top &&
		entity->positionVector.y + entity->radius > bottom &&
		entity->positionVector.z - entity->radius < nearBound &&
		entity->positionVector.z + entity->radius > farBound
	)
		return INTERSECT;
		*/
	// It's outside if the previous were false, logical :P
	return OUTSIDE;
}


/// Checks if the target tri is inside this Octree node, intersecting it or outside.
int CollissionShapeOctree::IsTriangleInside(Triangle * tri){
	// Assume the Octree is axis-aligned.
	Vector3f min, max;
	min = Vector3f::Minimum(Vector3f::Minimum(tri->point1, tri->point2), tri->point3);
	max = Vector3f::Maximum(Vector3f::Maximum(tri->point1, tri->point2), tri->point3);

	// Check if it's inside.
	if (max.x < right &&
		min.x > left &&
		max.y < top &&
		min.y > bottom &&
		max.z < nearBound &&
		min.z > farBound
		)
		return INSIDE;
	// It's outside if the previous were false, logical :P
	// .. or intersect, but same thing here I think.
	return OUTSIDE;
}
