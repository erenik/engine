// Emil Hedemalm
// 2013-03-17

#include "CollisionShapeOctree.h"
#include "Physics/PhysicsProperty.h"
#include "Physics/PhysicsManager.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/OpenGL.h"
#include "GraphicsState.h"
#include "Graphics/Camera/Camera.h"
#include "Collision.h"
#include "Viewport.h"

#include "Sphere.h"

/// Define the static values

/// Basic constructor
CollisionShapeOctree::CollisionShapeOctree()
{	
	// Default Entity pointer amounts.
	subdivision = 0;
	parent = NULL;
	optimized = false;

	// Stuff.
}

/// Internal constructor that sets the Entity's boundaries and subdivision level
CollisionShapeOctree::CollisionShapeOctree(float i_leftBound, float i_rightBound, float i_topBound, float i_bottomBound, float i_nearBound, float i_farBound, int i_subdivision)
{
	assert(i_leftBound <= i_rightBound);
	assert(i_topBound >= i_bottomBound);
	assert(i_nearBound >= i_farBound);

	// Set boundaries, calculate radius, etc.
	SetBoundaries(i_leftBound, i_rightBound, i_topBound, i_bottomBound, i_nearBound, i_farBound);
	subdivision = i_subdivision;
	
	optimized = false;
}

/// Default destructor that deallocates all children
CollisionShapeOctree::~CollisionShapeOctree(){
	children.ClearAndDelete();
}

/// Removes unused children. After optimizing no new triangles may be added.
void CollisionShapeOctree::Optimize()
{
	for (int i = 0; i < children.Size(); ++i)
	{
		/// If it has no triangles in it, just remove it.
		CollisionShapeOctree * child = children[i];
		if (child->RegisteredShapes() == 0)
		{
			children.Remove(child);
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
	if (children.Size() == 0)
	{
		/// Check sizes, if for example width, height or depth any approach a certain value, opt to instead perform a quad- or perhaps even a binary subdivision.
		if (height < width * 0.5f)
		{
			/// Ok, so height is low, skip height when subdividing, perform a quad-division.
			/// Hither left, hither right, farther left, farther right
			children.Add(new CollisionShapeOctree(left, center[0], top, bottom, nearBound, center[2], childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(center[0], right, top, bottom, nearBound, center[2], childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(left, center[0], top, bottom, center[2], farBound, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(center[0], right, top, bottom, center[2], farBound, childSubdivisionLvl));
			/// And the four on the axes, left, right, hither, farther
			children.Add(new CollisionShapeOctree(left, center[0], top, bottom, (center[2] + nearBound)*0.5f, (center[2] + farBound)*0.5f, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(center[0], right, top, bottom, (center[2] + nearBound)*0.5f, (center[2] + farBound)*0.5f, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree((center[0]+left)*0.5f, (center[0]+right)*0.5f, top, bottom, nearBound, center[2], childSubdivisionLvl));
			children.Add(new CollisionShapeOctree((center[0]+left)*0.5f, (center[0]+right)*0.5f, top, bottom, center[2], farBound, childSubdivisionLvl));
			// Center
			children.Add(new CollisionShapeOctree((center[0]+left)*0.5f, (center[0]+right)*0.5f, top, bottom, (center[2]+nearBound)*0.5f, (center[2]+farBound)*0.5f, childSubdivisionLvl));
		}
		/// Standard extended octree division here if all lengths (width, height, depth) are near equal in size.
		else {
			/// Standard variant octree subdivision with complementary compartments (25 in total).
			/// First primary octree division 8, in order:
			/// HITHER_LOWER_LEFT, HITHER_LOWER_RIGHT, HITHER_UPPER_LEFT, HITHER_UPPER_RIGHT, FARTHER_LOWER_LEFT, FARTHER_LOWER_RIGHT, FARTHER_UPPER_LEFT, FARTHER_UPPER_RIGHT.
			children.Add(new CollisionShapeOctree(left, center[0], center[1], bottom, nearBound, center[2], childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(center[0], right, center[1], bottom, nearBound, center[2], childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(left, center[0], top, center[1], nearBound, center[2], childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(center[0], right, top, center[1], nearBound, center[2], childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(left, center[0], center[1], bottom, center[2], farBound, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(center[0], right, center[1], bottom, center[2], farBound, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(left, center[0], top, center[1], center[2], farBound, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(center[0], right, top, center[1], center[2], farBound, childSubdivisionLvl));

			
			// UPPER_LEFT, UPPER_RIGHT, LOWER_LEFT, LOWER_RIGHT, FARTHER_DOWN, FARTHER_UP, HITHER_DOWN, HITHER_UP,
			children.Add(new CollisionShapeOctree(left, center[0], top, center[1], (center[2] + nearBound)*0.5f, (center[2] + farBound)*0.5f, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(center[0], right, top, center[1], (center[2] + nearBound)*0.5f, (center[2] + farBound)*0.5f, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(left, center[0], center[1], bottom, (center[2] + nearBound)*0.5f, (center[2] + farBound)*0.5f, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(center[0], right, center[1], bottom, (center[2] + nearBound)*0.5f, (center[2] + farBound)*0.5f, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree((center[0]+left)*0.5f, (center[0]+right)*0.5f, center[1], bottom, center[2], farBound, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree((center[0]+left)*0.5f, (center[0]+right)*0.5f, top, center[1], center[2], farBound, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree((center[0]+left)*0.5f, (center[0]+right)*0.5f, center[1], bottom, nearBound, center[2], childSubdivisionLvl));
			children.Add(new CollisionShapeOctree((center[0]+left)*0.5f, (center[0]+right)*0.5f, top, center[1], nearBound, center[2], childSubdivisionLvl));

			// Center
			children.Add(new CollisionShapeOctree((center[0]+left)*0.5f, (center[0]+right)*0.5f, (center[1]+top)*0.5f, (center[1]+bottom)*0.5f, (center[2]+nearBound)*0.5f, (center[2]+farBound)*0.5f, childSubdivisionLvl));

			// + Center-branches
			//HITHER, FARTHER, LEFTER, RIGHTER, UPPER, LOWER,
			children.Add(new CollisionShapeOctree((center[0]+left)*0.5f, (center[0]+right)*0.5f, (center[1]+top)*0.5f, (center[1]+bottom)*0.5f, nearBound, center[2], childSubdivisionLvl));
			children.Add(new CollisionShapeOctree((center[0]+left)*0.5f, (center[0]+right)*0.5f, (center[1]+top)*0.5f, (center[1]+bottom)*0.5f, center[2], farBound, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(left, center[0], (center[1]+top)*0.5f, (center[1]+bottom)*0.5f, (center[2]+nearBound)*0.5f, (center[2]+farBound)*0.5f, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(center[0], right, (center[1]+top)*0.5f, (center[1]+bottom)*0.5f, (center[2]+nearBound)*0.5f, (center[2]+farBound)*0.5f, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree((center[0]+left)*0.5f, (center[0]+right)*0.5f, top, center[1], (center[2]+nearBound)*0.5f, (center[2]+farBound)*0.5f, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree((center[0]+left)*0.5f, (center[0]+right)*0.5f, center[1], bottom, (center[2]+nearBound)*0.5f, (center[2]+farBound)*0.5f, childSubdivisionLvl));

			// + Huggers(apartments, whatever)
			// FATHER_LEFT, FARTHER_RIGHT, HITHER_LEFT, HITHER_RIGHT,
			children.Add(new CollisionShapeOctree(left, center[0], (center[1]+top)*0.5f, (center[1]+bottom)*0.5f, center[2], farBound, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(center[0], right, (center[1]+top)*0.5f, (center[1]+bottom)*0.5f, center[2], farBound, childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(left, center[0], (center[1]+top)*0.5f, (center[1]+bottom)*0.5f, nearBound, center[2], childSubdivisionLvl));
			children.Add(new CollisionShapeOctree(center[0], right, (center[1]+top)*0.5f, (center[1]+bottom)*0.5f, nearBound, center[2], childSubdivisionLvl));
		}
	}
	for (int i = 0; i < children.Size(); ++i)
		children[i]->parent = this;
	// Subdivide all children further if levels is still positive.
	if (levels > 0){
		for (int i = 0; i < children.Size(); ++i)
			children[i]->subdivide(levels);
	}
}

void CollisionShapeOctree::ClearAll()
{
	optimized = false;
	triangles.Clear();
	for (int i = 0; i < children.Size(); ++i)
	{
		children[i]->ClearAll();
		// Delete children for re-creation.
		children.ClearAndDelete();
	}
}


/** Adds a scenegraph Entity to this vfcOctree Entity unless max nodes has been reached.
	If MAX_INITIAL_NODES_BEFORE_SUBDIVISION is reached, subdivision hasn't yet been done and MAX_SUBDIVISION hasn't been reached, subdivision occurs.
*/
bool CollisionShapeOctree::AddTriangle(Triangle * tri)
{
	if (optimized)
	{
		assert(false && "optimized");
		return false;
	}
	// Check that it isn't already added!
	bool exists = triangles.Exists(tri);
	if (exists){
		std::cout<<"\nAdding Entity to CollisionShapeOctree while it already exists!";
		assert("Adding Entity to CollisionShapeOctree while it already exists!" && false);
		return false;
	}
	// If we have children, check if it fits in any of them.
	if (children.Size()){
		int result = OUTSIDE;
		for (int i = 0; i < children.Size(); ++i){
			result = children[i]->IsTriangleInside(tri);
			switch(result){
					// If the Entity is outside, just check next child-node~
				case OUTSIDE:
					continue; 	break;
					// If intersecting, don't do anything. Check all children and handle intersections after this for-loop.
				case INTERSECT: break;
					// If it is inside, continue down the chain and then return from hierrr.
				case INSIDE:
					return children[i]->AddTriangle(tri);
			}
		}
		// If we arrived here, it's either intersecting or something, so add it to our current children since it can't go further down the tree.
//		triangles.Add(tri);
	//	std::cout<<"\nTriangles in element: "<<triangles.Size();
//		return true;
	} /// End of trying to enter it into any of our children

	// Okay, no spot in children, check if we should subdivide it (if the children aren't already allocated, that is!)
	if (triangles.Size() > MAX_INITIAL_NODES_BEFORE_SUBDIVISION && children.Size() == 0)
	{
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
	for (int i = 0; i < children.Size(); ++i){
		if (children[i]->Exists(tri))
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
	for (int i = 0; i < children.Size(); ++i){
		if (children[i]->RemoveTriangle(tri))
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
	for (int i = 0; i < children.Size(); ++i){
		total += children[i]->RegisteredShapes();
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
	for (int i = 0; i < children.Size(); ++i){
		children[i]->PrintContents();
	}
}

/// To be used via the GraphicsManager's RenderPhysics function ONLY.
bool CollisionShapeOctree::Render(GraphicsState * graphicsState)
{
	// If the camera is inside here, render with hard mode-desu for clarity's sake.
	Vector3f cam = GraphicsThreadGraphicsState.camera->Position();
	bool isInside = false, wasInside = false;

	/// Check if inside any children, render only that child if so
	int childIndex = -1;
	// Render children first
	for (int i = 0; i < children.Size(); ++i){
		CollisionShapeOctree * child = children[i];
		if (cam[0] < child->right &&
			cam[0] > child->left &&
			cam[1] < child->top &&
			cam[1] > child->bottom &&
			cam[2] > child->farBound &&
			cam[2] < child->nearBound)
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
	for (int i = 0; i < children.Size(); ++i){
		/// Render, and if true (meaning it was inside), don't render ourselves or any more stuff
		if (children[i]->Render(graphicsState))
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
			glVertex3f(tri->point1[0], tri->point1[1], tri->point1[2]);
			glVertex3f(tri->point2[0], tri->point2[1], tri->point2[2]);
			glVertex3f(tri->point3[0], tri->point3[1], tri->point3[2]);
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
int CollisionShapeOctree::FindCollisions(EntitySharedPtr targetEntity, List<Collision> & collissionList, int entrySubdivisionLevel){
	Matrix4f identityMatrix;
	return FindCollisions(targetEntity, collissionList, identityMatrix);
}

/** Searches for collissions with specified entity.
	If entry subdivision level is not specified the initial call will set it automatically (used for recursion limits)
	Returns amount of collissions tested.
	- localTransform is applied to all relevant triangle upon testing if provided.
*/
int CollisionShapeOctree::FindCollisions(EntitySharedPtr targetEntity, List<Collision> & collissionList, Matrix4f & localTransform, int entrySubdivisionLevel/* = -1*/)
{
	if (entrySubdivisionLevel == -1)
		entrySubdivisionLevel = this->subdivision;

	GraphicsState * graphicsState = graphicsState;

	int collissionsTested = 0;
	/// First it's current node:
	for (int i = 0; i < triangles.Size(); ++i)
	{
		Triangle * trianglePointer = triangles[i];
//		assert(trianglePointer && "Nullentity in CollisionShapeOctree::FindCollisions");
		Triangle tri = *trianglePointer;
//		assert(trianglePointer->normal.MaxPart());
		tri.Transform(localTransform);
		// Probably a bad matrix one frame.. just skip it.
		if (!tri.normal.MaxPart())
			continue;
		assert(tri.normal.MaxPart());
		/* // Collisions are not viewport-dependant, so re-write this later if need to debug again.
		Viewport * viewport = GraphicsThreadGraphicsState.activeViewport;
		if (viewport)
		{
			if (viewport->renderPhysics && viewport->renderCollisionTriangles)
				Physics.activeTriangles.Add(tri);
		}*/
		Collision col;
		// Check physics type. If Entity has physics type mesh then we should try and optimize it in some other way.
		if (targetEntity->physics->type == ShapeType::MESH){
			std::cout<<"\nWARNING: Entity of type mesh trying to collide with a single Triangle! Is this the intent?";
			assert(targetEntity->physics->type != ShapeType::MESH && "CollisionShapeOctree::FindCollisions mesh-triangle collission where unwanted!");
			return collissionsTested;
		}
		// Entity shape-type!
		switch(targetEntity->physics->shapeType)
		{
			case ShapeType::SPHERE: 
			{
				Sphere sphere; // = (Sphere*)targetEntity->physics->shape;
				sphere.position = targetEntity->worldPosition;
				sphere.radius = targetEntity->physics->physicalRadius;
				if (TriangleSphereCollision(&tri, &sphere, col, false))
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
		for (int i = 0; i < children.Size(); ++i){
			CollisionShapeOctree * ch = children[i];
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
		for (int i = 0; i < children.Size(); ++i){
			CollisionShapeOctree * child = children[i];
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
int CollisionShapeOctree::IsEntityInside(EntitySharedPtr entity, Matrix4f & localTransform)
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
		distance = plane->Distance(entity->worldPosition);
		if (distance < -radius)
			return Loc::OUTSIDE;
		else if (distance < radius)
			result =  Loc::INTERSECT;
	}
	return(result);


	/// The below approach doesn't work when the transform includes rotation.
	/*
	
	float entityLeft = entity->position[0] - entity->Radius(),
		entityRight = entity->position[0] + entity->Radius(),
		entityTop = entity->position[1] + entity->Radius(),
		entityBottom = entity->position[1] - entity->Radius(),
		entityNear = entity->position[2] + entity->Radius(),
		entityFar = entity->position[2] - entity->Radius();
	// Check if it's inside.
	if (entityRight < maxVec[0] &&
		entityLeft > minVec[0] &&
		entityTop < maxVec[1] &&
		entityBottom > minVec[1] &&
		entityNear < maxVec[2] &&
		entityFar > minVec[2]
		)
		return INSIDE;
	// Or intersecting, just compare with inverted radius
	else if (entityLeft < maxVec[0] &&
		entityRight > minVec[0] &&
		entityBottom < maxVec[1] &&
		entityTop > minVec[1] &&
		entityFar < maxVec[2] &&
		entityNear > minVec[2]
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
	if (max[0] < right &&
		min[0] > left &&
		max[1] < top &&
		min[1] > bottom &&
		max[2] < nearBound &&
		min[2] > farBound
		)
		return INSIDE;
	// It's outside if the previous were false, logical :P
	// .. or intersect, but same thing here I think.
	return OUTSIDE;
}
