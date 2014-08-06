// Emil Hedemalm
// 2013-03-17
#include "CollisionShapeOctree.h"
#include "Physics/PhysicsProperty.h"
#include "Physics/PhysicsManager.h"
#include "Graphics/GraphicsManager.h"
#include <GL/glew.h>
#include "GraphicsState.h"
#include "Graphics/Camera/Camera.h"
#include "Collision.h"
#include "Viewport.h"

#include "PhysicsLib/Shapes/Sphere.h"

/// Define the static values

/// Basic constructor
CollisionShapeOctree::CollisionShapeOctree()
{	
	// Default Entity pointer amounts.
	subdivision = 0;
	parent = NULL;
	optimized = false;

	// Stuff.
	MAX_INITIAL_NODES_BEFORE_SUBDIVISION = 7;
}

/// Internal constructor that sets the Entity's boundaries and subdivision level
CollisionShapeOctree::CollisionShapeOctree(float i_leftBound, float i_rightBound, float i_topBound, float i_bottomBound, float i_nearBound, float i_farBound, int i_subdivision)
{
	assert(i_leftBound < i_rightBound);
	assert(i_topBound > i_bottomBound);
	assert(i_nearBound > i_farBound);

	// Set boundaries, calculate radius, etc.
	SetBoundaries(i_leftBound, i_rightBound, i_topBound, i_bottomBound, i_nearBound, i_farBound);
	subdivision = i_subdivision;
	
	optimized = false;
}

/// Default destructor that deallocates all children
CollisionShapeOctree::~CollisionShapeOctree(){
	childList.ClearAndDelete();
}

/// Removes unused children. After optimizing now new triangles may be added.
void CollisionShapeOctree::Optimize()
{
	for (int i = 0; i < childList.Size(); ++i)
	{
		/// If it has no triangles in it, just remove it.
		CollisionShapeOctree * child = childList[i];
		if (child->RegisteredShapes() == 0)
		{
			childList.Remove(child);
			delete child;
			--i;
		}
		/// If not, call optimize on it.
		else {
			child->Optimize();
		}
	}
	optimized = true;
}

/// Sets boundaries of the top-level node.
void CollisionShapeOctree::SetBoundaries(float leftBound, float rightBound, float topBound, float bottomBound, float i_nearBound, float i_farBound){
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
	width = (right - left) / 2.0f;
	height = (top - bottom) / 2.0f;
	depth = (nearBound - farBound) / 2.0f;
	
	/// Fetch extremees and radius too.
	min = Vector3f(left, bottom, farBound);
	max = Vector3f(right, top, nearBound);
	/// Radius important!
	radius = (max - center).Length();
}


/** Recursive subdivision function that subdivides the nodes down the tree specified amount of levels.
	If MAX_SUBDIVISION is passed an exception will be thrown.
*/
void CollisionShapeOctree::subdivide(int levels)
{
	// Decrement levels
	levels--;
	// Check that the maximum subdivision level hasn't been reached.
	int childSubdivisionLvl = subdivision + 1;
	if (childSubdivisionLvl > MAX_SUBDIVISION)
		throw 4;
	// Check if the children aren't already allocated.
	if (childList.Size() == 0)
	{
		/// Check sizes, if for example width, height or depth any approach a certain value, opt to instead perform a quad- or perhaps even a binary subdivision.
		if (height < width * 0.5f)
		{
			/// Ok, so height is low, skip height when subdividing, perform a quad-division.
			/// Hither left, hither right, farther left, farther right
			childList.Add(new CollisionShapeOctree(left, center.x, top, bottom, nearBound, center.z, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(center.x, right, top, bottom, nearBound, center.z, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(left, center.x, top, bottom, center.z, farBound, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(center.x, right, top, bottom, center.z, farBound, childSubdivisionLvl));
			/// And the four on the axes, left, right, hither, farther
			childList.Add(new CollisionShapeOctree(left, center.x, top, bottom, (center.z + nearBound)*0.5f, (center.z + farBound)*0.5f, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(center.x, right, top, bottom, (center.z + nearBound)*0.5f, (center.z + farBound)*0.5f, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, top, bottom, nearBound, center.z, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, top, bottom, center.z, farBound, childSubdivisionLvl));
			// Center
			childList.Add(new CollisionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, top, bottom, (center.z+nearBound)*0.5f, (center.z+farBound)*0.5f, childSubdivisionLvl));
		}
		/// Standard extended octree division here if all lengths (width, height, depth) are near equal in size.
		else {
			/// Standard variant octree subdivision with complementary compartments (25 in total).
			/// First primary octree division 8, in order:
			/// HITHER_LOWER_LEFT, HITHER_LOWER_RIGHT, HITHER_UPPER_LEFT, HITHER_UPPER_RIGHT, FARTHER_LOWER_LEFT, FARTHER_LOWER_RIGHT, FARTHER_UPPER_LEFT, FARTHER_UPPER_RIGHT.
			childList.Add(new CollisionShapeOctree(left, center.x, center.y, bottom, nearBound, center.z, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(center.x, right, center.y, bottom, nearBound, center.z, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(left, center.x, top, center.y, nearBound, center.z, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(center.x, right, top, center.y, nearBound, center.z, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(left, center.x, center.y, bottom, center.z, farBound, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(center.x, right, center.y, bottom, center.z, farBound, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(left, center.x, top, center.y, center.z, farBound, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(center.x, right, top, center.y, center.z, farBound, childSubdivisionLvl));

			
			// UPPER_LEFT, UPPER_RIGHT, LOWER_LEFT, LOWER_RIGHT, FARTHER_DOWN, FARTHER_UP, HITHER_DOWN, HITHER_UP,
			childList.Add(new CollisionShapeOctree(left, center.x, top, center.y, (center.z + nearBound)*0.5f, (center.z + farBound)*0.5f, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(center.x, right, top, center.y, (center.z + nearBound)*0.5f, (center.z + farBound)*0.5f, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(left, center.x, center.y, bottom, (center.z + nearBound)*0.5f, (center.z + farBound)*0.5f, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(center.x, right, center.y, bottom, (center.z + nearBound)*0.5f, (center.z + farBound)*0.5f, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, center.y, bottom, center.z, farBound, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, top, center.y, center.z, farBound, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, center.y, bottom, nearBound, center.z, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, top, center.y, nearBound, center.z, childSubdivisionLvl));

			// Center
			childList.Add(new CollisionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, (center.y+top)*0.5f, (center.y+bottom)*0.5f, (center.z+nearBound)*0.5f, (center.z+farBound)*0.5f, childSubdivisionLvl));

			// + Center-branches
			//HITHER, FARTHER, LEFTER, RIGHTER, UPPER, LOWER,
			childList.Add(new CollisionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, (center.y+top)*0.5f, (center.y+bottom)*0.5f, nearBound, center.z, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, (center.y+top)*0.5f, (center.y+bottom)*0.5f, center.z, farBound, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(left, center.x, (center.y+top)*0.5f, (center.y+bottom)*0.5f, (center.z+nearBound)*0.5f, (center.z+farBound)*0.5f, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(center.x, right, (center.y+top)*0.5f, (center.y+bottom)*0.5f, (center.z+nearBound)*0.5f, (center.z+farBound)*0.5f, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, top, center.y, (center.z+nearBound)*0.5f, (center.z+farBound)*0.5f, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree((center.x+left)*0.5f, (center.x+right)*0.5f, center.y, bottom, (center.z+nearBound)*0.5f, (center.z+farBound)*0.5f, childSubdivisionLvl));

			// + Huggers(apartments, whatever)
			// FATHER_LEFT, FARTHER_RIGHT, HITHER_LEFT, HITHER_RIGHT,
			childList.Add(new CollisionShapeOctree(left, center.x, (center.y+top)*0.5f, (center.y+bottom)*0.5f, center.z, farBound, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(center.x, right, (center.y+top)*0.5f, (center.y+bottom)*0.5f, center.z, farBound, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(left, center.x, (center.y+top)*0.5f, (center.y+bottom)*0.5f, nearBound, center.z, childSubdivisionLvl));
			childList.Add(new CollisionShapeOctree(center.x, right, (center.y+top)*0.5f, (center.y+bottom)*0.5f, nearBound, center.z, childSubdivisionLvl));
		}
	}
	for (int i = 0; i < childList.Size(); ++i)
		childList[i]->parent = this;
	// Subdivide all children further if levels is still positive.
	if (levels > 0){
		for (int i = 0; i < childList.Size(); ++i)
			childList[i]->subdivide(levels);
	}
}

void CollisionShapeOctree::clearAll(){
	triangles.Clear();
	for (int i = 0; i < childList.Size(); ++i)
		childList[i]->clearAll();
}


/** Adds a scenegraph Entity to this vfcOctree Entity unless max nodes has been reached.
	If MAX_INITIAL_NODES_BEFORE_SUBDIVISION is reached, subdivision hasn't yet been done and MAX_SUBDIVISION hasn't been reached, subdivision occurs.
*/
bool CollisionShapeOctree::AddTriangle(Triangle * tri)
{
	if (optimized)
		return false;
	// Check that it isn't already added!
	bool exists = triangles.Exists(tri);
	if (exists){
		std::cout<<"\nAdding Entity to CollisionShapeOctree while it already exists!";
		assert("Adding Entity to CollisionShapeOctree while it already exists!" && false);
		return false;
	}
	// If we have children, check if it fits in any of them.
	if (childList.Size()){
		int result = OUTSIDE;
		for (int i = 0; i < childList.Size(); ++i){
			result = childList[i]->IsTriangleInside(tri);
			switch(result){
					// If the Entity is outside, just check next child-node~
				case OUTSIDE:
					continue; 	break;
					// If intersecting, don't do anything. Check all children and handle intersections after this for-loop.
				case INTERSECT: break;
					// If it is inside, continue down the chain and then return from hierrr.
				case INSIDE:
					return childList[i]->AddTriangle(tri);
			}
		}
		// If we arrived here, it's either intersecting or something, so add it to our current children since it can't go further down the tree.
		triangles.Add(tri);
	//	std::cout<<"\nTriangles in element: "<<triangles.Size();
		return true;
	} /// End of trying to enter it into any of our children

	// Okay, no spot in children, check if we should subdivide it (if the children aren't already allocated, that is!)
	if (triangles.Size() > MAX_INITIAL_NODES_BEFORE_SUBDIVISION && childList.Size() == NULL){
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
bool CollisionShapeOctree::Exists(Triangle * tri){
	if (triangles.Exists(tri))
		return true;
	for (int i = 0; i < childList.Size(); ++i){
		if (childList[i]->Exists(tri))
			return true;
	}
	return false;
}

/// Removes the Entity and re-inserts it to it's new location
bool CollisionShapeOctree::RepositionTriangle(Triangle * tri){
	RemoveTriangle(tri);
	AddTriangle(tri);
	return true;
}

// Removes a scenegraph Entity from this vfcOctree. Searches recursively until.
bool CollisionShapeOctree::RemoveTriangle(Triangle * tri){
	if (triangles.Remove(tri)){
		return true;
	}
	// Go through all children. If one of them finds the target Entity, return true without processing any more children.
	for (int i = 0; i < childList.Size(); ++i){
		if (childList[i]->RemoveTriangle(tri))
			return true;
	}
	if (this->subdivision == 0){
		assert(this->Exists(tri));
		assert(false && "Unable to remove entity in CollisionShapeOctree::RemoveEntity!");
	}
	return false;
}

/// Returns a count of all items within this element, including any children it possesses.
int CollisionShapeOctree::RegisteredShapes(){
	int total = 0;
	total += triangles.Size();
	for (int i = 0; i < childList.Size(); ++i){
		total += childList[i]->RegisteredShapes();
	}
	return total;
}

/// Prints contents in each cubicle
void CollisionShapeOctree::PrintContents()
{
	std::cout<<"\nCollisionShapeOctree: ";
	String spacing;
	for (int i = 0; i < subdivision; ++i){
		spacing += " ";
	}
	std::cout<<spacing;
	std::cout<<"- Triangles: "<<triangles.Size();

	for (int i = 0; i < triangles.Size(); ++i)
	{
		Triangle * triangle = triangles[i];
		assert(triangle->normal.MaxPart());
	}

	/// Print children if any
	for (int i = 0; i < childList.Size(); ++i){
		childList[i]->PrintContents();
	}
}

/// To be used via the GraphicsManager's RenderPhysics function ONLY.
bool CollisionShapeOctree::Render(GraphicsState * graphicsState)
{
	// If the camera is inside here, render with hard mode-desu for clarity's sake.
	Vector3f cam = graphicsState->camera->Position();
	bool isInside = false, wasInside = false;

	/// Check if inside any children, render only that child if so
	int childIndex = -1;
	// Render children first
	for (int i = 0; i < childList.Size(); ++i){
		CollisionShapeOctree * child = childList[i];
		if (cam.x < child->right &&
			cam.x > child->left &&
			cam.y < child->top &&
			cam.y > child->bottom &&
			cam.z > child->farBound &&
			cam.z < child->nearBound)
			childIndex = i;
		break;
	}
/*
	if (childIndex != -1){
		child[childIndex]->Render();
		return true;
	}
*/
//	if (isInside)
//		glDepthMask(GL_TRUE);


	// Render children first
	for (int i = 0; i < childList.Size(); ++i){
		/// Render, and if true (meaning it was inside), don't render ourselves or any more stuff
		if (childList[i]->Render(graphicsState))
			wasInside = true;
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
int CollisionShapeOctree::FindCollisions(Entity * targetEntity, List<Collision> & collissionList, int entrySubdivisionLevel){
	Matrix4f identityMatrix;
	return FindCollisions(targetEntity, collissionList, identityMatrix);
}

/** Searches for collissions with specified entity.
	If entry subdivision level is not specified the initial call will set it automatically (used for recursion limits)
	Returns amount of collissions tested.
	- localTransform is applied to all relevant triangle upon testing if provided.
*/
int CollisionShapeOctree::FindCollisions(Entity * targetEntity, List<Collision> & collissionList, Matrix4f & localTransform, int entrySubdivisionLevel/* = -1*/)
{
	if (entrySubdivisionLevel == -1)
		entrySubdivisionLevel = this->subdivision;

	GraphicsState * graphicsState = Graphics.graphicsState;

	int collissionsTested = 0;
	/// First it's current node:
	for (int i = 0; i < triangles.Size(); ++i)
	{
		Triangle * trianglePointer = triangles[i];
		assert(trianglePointer && "Nullentity in CollisionShapeOctree::FindCollisions");
		Triangle tri = *trianglePointer;
		assert(trianglePointer->normal.MaxPart());
		tri.Transform(localTransform);
		assert(tri.normal.MaxPart());
		Viewport * viewport = ActiveViewport;
		if (viewport)
		{
			if (viewport->renderPhysics && viewport->renderCollisionTriangles)
				Physics.activeTriangles.Add(tri);
		}
		Collision col;
		// Check physics type. If Entity has physics type mesh then we should try and optimize it in some other way.
		if (targetEntity->physics->type == ShapeType::MESH){
			std::cout<<"\nWARNING: Entity of type mesh trying to collide with a single Triangle! Is this the intent?";
			assert(targetEntity->physics->type != ShapeType::MESH && "CollisionShapeOctree::FindCollisions mesh-triangle collission where unwanted!");
			return collissionsTested;
		}
		// Entity shape-type!
		switch(targetEntity->physics->physicsShape)
		{
			case ShapeType::SPHERE: 
			{
				Sphere sphere; // = (Sphere*)targetEntity->physics->shape;
				sphere.position = targetEntity->position;
				sphere.radius = targetEntity->physics->physicalRadius;
				if (TriangleSphereCollision(&tri, &sphere, col))
				{
				//	col.distanceIntoEachOther
				//	col.one = targetEntity;
				//	col.two =
					collissionList.Add(col);
					Physics.activeTriangles.Add(tri);
				}
				break;
			}
			default:
				std::cout<<"\nUnsupported shape-type for specified mesh when checking collissions vs. Triangles in CollisionShapeOctree::FindCollisions!";
				assert(false && "Unsupported shape-type for specified mesh when checking collissions vs. Triangles in CollisionShapeOctree::FindCollisions!");
				break;
		}
		++collissionsTested;
	}

	/// If entry node, begin all recursion.
	if (this->subdivision == entrySubdivisionLevel){
		/// Then all children
		for (int i = 0; i < childList.Size(); ++i){
			CollisionShapeOctree * ch = childList[i];
			/// Do the actual culling with this continue-statement, yo!
			if (ch->IsEntityInside(targetEntity, localTransform) == OUTSIDE)
				continue;
			collissionsTested += ch->FindCollisions(targetEntity, collissionList, localTransform, entrySubdivisionLevel);
		}
		/// And at last all parents
		if (parent && this->subdivision <= entrySubdivisionLevel)
			collissionsTested += parent->FindCollisions(targetEntity, collissionList, localTransform, entrySubdivisionLevel);
		return collissionsTested;
	}
	/// Process children if subdivision level is higher (further down the tree.)
	else if (this->subdivision > entrySubdivisionLevel){
		/// Then all children
		for (int i = 0; i < childList.Size(); ++i){
			CollisionShapeOctree * child = childList[i];
			/// Do the actual culling with this continue-statement, yo!
			if (child->IsEntityInside(targetEntity, localTransform) == OUTSIDE)
				continue;
			collissionsTested += child->FindCollisions(targetEntity, collissionList, localTransform, entrySubdivisionLevel);
		}
		return collissionsTested;
	}
	/// And process parents only if lesser subdivision
	else if (this->subdivision < entrySubdivisionLevel){
		/// Then go to next parent.
		if (parent && this->subdivision <= entrySubdivisionLevel)
			collissionsTested += parent->FindCollisions(targetEntity, collissionList, localTransform, entrySubdivisionLevel);
		return collissionsTested;
	}
	assert(false && "CollisionShapeOctree::FindCollisions at a point where it shouldn't be!");
	return 0;
}

/// Checks if the target Entity is inside this CollisionShapeOctree Entity, intersecting it or outside.
int CollisionShapeOctree::IsEntityInside(Entity * entity, Matrix4f & localTransform)
{
	/// Transform the vectors to the local transform position.
	Vector4f minVec(left, bottom, farBound, 1.0f);
	Vector4f maxVec(right, top, nearBound, 1.0f);
	minVec = localTransform * minVec;
	maxVec = localTransform * maxVec;

	float midX = (left + right) * 0.5f;
	float midY = (top + bottom) * 0.5f;
	Vector4f hitherCenter(midX, midY, nearBound, 1.f);
	hitherCenter = localTransform * hitherCenter;

	Vector3f hitherTopLeft(left, top, nearBound), 
		hitherTopRight(right, top, nearBound), 
		hitherBottomLeft(left, bottom, nearBound), 
		hitherBottomRight(right, bottom, nearBound);
	Vector3f fartherTopLeft( left, top, farBound), 
		fartherTopRight(right, top, farBound), 
		fartherBottomLeft(left, bottom, farBound), 
		fartherBottomRight(right, bottom, farBound);
	
	// Gather the 6 planes.
	Plane planes[6];
	// Same code as in Frustum class, pretty much.
	planes[LEFT_PLANE].Set3Points(hitherTopLeft, hitherBottomLeft, fartherBottomLeft);
	planes[RIGHT_PLANE].Set3Points(hitherTopRight, fartherTopRight, fartherBottomRight);
	planes[BOTTOM_PLANE].Set3Points(fartherBottomRight, fartherBottomLeft, hitherBottomLeft);
	planes[TOP_PLANE].Set3Points(fartherTopRight, hitherTopRight, hitherTopLeft);
	planes[NEAR_PLANE].Set3Points(hitherBottomLeft, hitherTopLeft, hitherTopRight);
	planes[FAR_PLANE].Set3Points(fartherBottomLeft, fartherBottomRight, fartherTopRight);

	// Transform them according to the transform.
	for (int i = 0; i < 6; ++i)
	{
		Plane & plane = planes[i];
		plane = plane.Transform(localTransform);
	}

	// Finally check stuff.
	float distance;
	int result = Loc::INSIDE;

	float radius = entity->physics->physicalRadius;

	for(int i=0; i < 6; i++) 
	{
		Plane * plane = &planes[i];
		distance = plane->Distance(entity->position);
		if (distance < -radius)
			return Loc::OUTSIDE;
		else if (distance < radius)
			result =  Loc::INTERSECT;
	}
	return(result);


	/// The below approach doesn't work when the transform includes rotation.
	/*
	
	float entityLeft = entity->position.x - entity->radius,
		entityRight = entity->position.x + entity->radius,
		entityTop = entity->position.y + entity->radius,
		entityBottom = entity->position.y - entity->radius,
		entityNear = entity->position.z + entity->radius,
		entityFar = entity->position.z - entity->radius;
	// Check if it's inside.
	if (entityRight < maxVec.x &&
		entityLeft > minVec.x &&
		entityTop < maxVec.y &&
		entityBottom > minVec.y &&
		entityNear < maxVec.z &&
		entityFar > minVec.z
		)
		return INSIDE;
	// Or intersecting, just compare with inverted radius
	else if (entityLeft < maxVec.x &&
		entityRight > minVec.x &&
		entityBottom < maxVec.y &&
		entityTop > minVec.y &&
		entityFar < maxVec.z &&
		entityNear > minVec.z
		)
		return INTERSECT;
	// It's outside if the previous were false, logical :P
	*/
	return OUTSIDE;
}


/// Checks if the target tri is inside this Octree node, intersecting it or outside.
int CollisionShapeOctree::IsTriangleInside(Triangle * tri){
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
