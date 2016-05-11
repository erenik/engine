// Emil Hedemalm
// 2013-03-17

#include "Mesh/Mesh.h"

#include "File/LogFile.h"
#include "../Material.h"
#include "Model/Model.h"
#include "Entity.h"
#include "Graphics/GraphicsProperty.h"
#include "Graphics/CompactGraphics.h"

#include "Texture.h"
#include "Physics/PhysicsManager.h"
#include "Physics/CompactPhysics.h"
#include "Physics/PhysicsProperty.h"

#include "Pathfinding/PathfindingProperty.h"
#include "Entity/EntityProperty.h"
#include "CompactEntity.h"
#include "../GraphicsState.h"
#include "Texture.h"
#include "Shader.h"
#include "TextureManager.h"
#include "Model/ModelManager.h"
#include "Script/ScriptProperty.h"
#include "Physics/Calc/EntityPhysicsEstimator.h"
#include <cstring>
#include "Graphics/GraphicsManager.h"
#include "Graphics/Fonts/TextFont.h"
#include "EntityProperty.h"

#include "PhysicsLib/Shapes/AABB.h"

#define UPDATE_SCALING_MATRIX \
	scalingMatrix.element[0] = scale.x;\
	scalingMatrix.element[5] = scale.y;\
	scalingMatrix.element[10] = scale.z;\
	if (scale.x != 1 || scale.y != 1 || scale.z != 1)\
		relevantScale = true;

const Material Entity::defaultMaterial = Material();

/// Creates a compact entity out of this Entity object
void Entity::CreateCompactEntity(CompactEntity * cEntity)
{
	assert(cEntity);
	strcpy(cEntity->name, name.c_str());
	strcpy(cEntity->model, model->RelativePath().c_str());
	if (diffuseMap)
		strcpy(cEntity->diffuseMap, diffuseMap->RelativePath().c_str());
	if (specularMap)
		strcpy(cEntity->specularMap, specularMap->RelativePath().c_str());
	if (normalMap)
		strcpy(cEntity->normalMap, normalMap->RelativePath().c_str());
	cEntity->position = localPosition;
	cEntity->scale = scale;
	cEntity->rotation = rotation;
	if (physics){
		if (cEntity->cPhysics)
			delete cEntity->cPhysics;
		cEntity->cPhysics = new CompactPhysics(physics);
	}
	if (graphics){
		if (cEntity->cGraphics)
			delete cEntity->cGraphics;
		cEntity->cGraphics = new CompactGraphics(graphics);
	}
}

/// Loads data from the file compact entity format
void Entity::LoadCompactEntityData(CompactEntity * cEntity)
{
	/// Model and texture is extracted elsewhere, but we can copy name, and other details here..
	name = cEntity->name;
	localPosition = cEntity->position;
	scale = cEntity->scale;
	rotation = cEntity->rotation;
	if (cEntity->cPhysics){
		assert(!physics);
		physics = new PhysicsProperty(cEntity->cPhysics);
		Physics.QueueMessage(new PMSetPhysicsType(this, physics->type));
		Physics.QueueMessage(new PMSetPhysicsShape(this, physics->shapeType));
	}
	if (cEntity->cGraphics){
		assert(!graphics);
		graphics = new GraphicsProperty(this);
		graphics->LoadDataFrom(cEntity->cGraphics);
	}
	diffuseMap = TexMan.GetTextureBySource(cEntity->diffuseMap);
	specularMap = TexMan.GetTextureBySource(cEntity->specularMap);
	normalMap = TexMan.GetTextureBySource(cEntity->normalMap);
	model = ModelMan.GetModel(cEntity->model);
	/// Recalculate model matrix after position update :P
	RecalculateMatrix();
}

Entity::Entity(int i_id)
{
	map = 0;
	inheritPositionOnly = false;
	relevantScale = false;
	sharedProperties = false;
	updateChildrenOnTransform = false;
	localPosition = Vector3f(0,0,0);
	scale = Vector3f(1,1,1);
	rotation = Vector3f(0,0,0);
	flags = 0;
	diffuseMap = NULL;
	specularMap = NULL;
	normalMap = NULL;
	emissiveMap = NULL;
	material = new Material(defaultMaterial);
	model = NULL;
	id = i_id;
	/// Owner o-o
	this->player = NULL;

	aabb = new AABB();

	// Default all flags to 0.
	/// Status, for whether it's part of rendering, physics, etc.
	this->registeredForRendering = false;
	this->registeredForPhysics = false;
	this->flaggedForDeletion = false;

	/// Nullify all pointers to add-ons. IMPORTANT!
	this->graphics = NULL;
	this->physics = NULL;
	this->scripts = NULL;
	/// Create it automatiaclly so we don't have to, cheers..
	this->pathfindingProperty = new PathfindingProperty(this);

	parent = NULL;
	cameraFocus = NULL;
	hasRotated = false;

	/// Calculate look-at vectors, etc...
	RecalculateMatrix(ALL_PARTS);

	/// Default render transform..
	renderTransform = &transformationMatrix;
	renderPosition = &worldPosition;
}

/// Default constructor...
Entity::~Entity()
{
	if (name.Contains("Boss1_"))
		int lall=5;
	LogMain("Deleting entity "+name+".", EXTENSIVE_DEBUG);
	/// Delete safe stuff.
	Delete();
	/// Delete those things that should have been deleted elsewhere too.
#ifndef SAFE_DELETE
#define SAFE_DELETE(p) {if(p) delete p; p = NULL; }
#endif
	SAFE_DELETE(graphics);
	SAFE_DELETE(physics);
	SAFE_DELETE(scripts);
	SAFE_DELETE(pathfindingProperty);
	properties.ClearAndDelete();
	if (material)
		delete material;
	material = NULL;
	// Models and textures will be deallocated by their respectice managers!
	SAFE_DELETE(aabb);
}

/// Removes links to parents/children as needed, prepares for deletion. Take care to call from render/physics thread.
void Entity::RemoveLinks()
{
	if (parent)
		parent->children.RemoveItem(this);

	for (int i = 0; i < children.Size(); ++i)
	{
		Entity * child = children[i];
		child->parent = 0;
	}
	children.Clear();
	parent = 0;
}

/// If reacting to collisions,.. pokes all properties about it too.
void Entity::OnCollision(Collision & data)
{
	for (int i = 0; i < properties.Size(); ++i)
	{
		EntityProperty * prop = properties[i];
		prop->OnCollision(data);
	}
}

void Entity::OnCollisionCallback(CollisionCallback *cc)
{
	for (int i = 0; i < properties.Size(); ++i)
	{
		EntityProperty * prop = properties[i];
		prop->OnCollisionCallback(cc);
	}
}


/// Fetches an AABB encapsulating this entity, including any children?
AABB * Entity::GetAABB()
{
	if (!aabb)
		aabb = new AABB();
	return aabb;
}


/// Deallocates additional points as needed.
void Entity::Delete()
{
}

/// Getter.
EntityProperty * Entity::GetProperty(String byName)
{
	for (int i = 0; i < properties.Size(); ++i)
	{
		EntityProperty * prop = properties[i];
		if (prop->name == byName)
			return prop;
	}
	return NULL;
}

/// Getter.
EntityProperty * Entity::GetProperty(int byID)
{
	for (int i = 0; i < properties.Size(); ++i)
	{
		EntityProperty * prop = properties[i];
		if (prop->GetID() == byID)
			return prop;
	}
	return NULL;

}

/// Is then re-directed to the properties in most cases.
void Entity::ProcessMessage(Message * message)
{
	for (int i = 0; i < properties.Size(); ++i)
	{
		EntityProperty * prop = properties[i];
		prop->ProcessMessage(message);
	}
}



/** Buffers this entities' models into graphics memory.
	Should only be used by the graphics manager. USE WITH CAUTION.
*/
void Entity::Bufferize()
{
	if (model)
		model->BufferizeIfNeeded();
	if (diffuseMap)
		diffuseMap->Bufferize();
	if (specularMap)
		specularMap->Bufferize();
	if (normalMap)
		normalMap->Bufferize();
	if (emissiveMap)
		emissiveMap->Bufferize();
}

void Entity::RenderOld(GraphicsState & graphicsState)
{
	Shader * shader = ActiveShader();
	int error = 0;
	// Bind texture if it isn't already bound.
	if (diffuseMap && graphicsState.currentTexture != diffuseMap){
		// When rendering an objectwith this program.
		glActiveTexture(GL_TEXTURE0 + 0);
		// Bind texture
		glBindTexture(GL_TEXTURE_2D, diffuseMap->glid);
		error = glGetError();

		// Set sampler in client graphicsState
		if (shader && shader->uniformBaseTexture != -1)
			glUniform1i(shader->uniformBaseTexture, 0);		// Sets sampler
		// Bind sampler
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		graphicsState.currentTexture = diffuseMap;
		glEnable(GL_TEXTURE_2D);
		glColor4f(1,1,1,1);
	}
	else if (diffuseMap == 0)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		graphicsState.currentTexture = 0;
	}

	error = glGetError();
	// Assign material
	// glProgramUniform Core since version 	4.1!
	// Use glUniform! Core since 2.0
	if (shader){
		glUniform4f(shader->uniformMaterial.ambientVec4, material->ambient[0], material->ambient[1], material->ambient[2], material->ambient[3]);
		glUniform4f(shader->uniformMaterial.diffuseVec4, material->diffuse[0], material->diffuse[1], material->diffuse[2], material->diffuse[3]);
		glUniform4f(shader->uniformMaterial.specularVec4, material->specular[0], material->specular[1], material->specular[2], material->specular[3]);
		glUniform1i(shader->uniformMaterial.shininessInt, material->shininess);
	}

	error = glGetError();
	// Save old matrix to the stack
	Matrix4d tmp = graphicsState.modelMatrixD;

	// Apply transformation
	graphicsState.modelMatrixD.Multiply(transformationMatrix);
	graphicsState.modelMatrixF = graphicsState.modelMatrixD;
	Matrix4f modelView = graphicsState.viewMatrixF * graphicsState.modelMatrixF;
	// Set uniform matrix in shader to point to the AppState modelView matrix.
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelView.getPointer());
	if (shader)
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, graphicsState.modelMatrixF.getPointer());
	error = glGetError();

	// Set texture
//	if (texture && texture->glid){
//		glBindTexture(GL_TEXTURE_2D, texture->glid);
//	}

	// Render the model, old-school
	Mesh * mesh = model->GetTriangulatedMesh();
	for (int i = 0; i < mesh->faces.Size(); ++i)
	{
		glBegin(GL_TRIANGLES);
		MeshFace * face = &mesh->faces[i];
		if (face->numVertices > 3){
			std::cout<<"\nmesh face with more than 3 numVertices D:";
		}
		for (int j = 0; j < 3; ++j){
			Vector3f position = Vector3f(mesh->vertices[face->vertices[j]][0], mesh->vertices[face->vertices[j]][1], mesh->vertices[face->vertices[j]][2]);
			position = graphicsState.modelMatrixF * position;
			position = graphicsState.viewMatrixF * position;
			position = graphicsState.projectionMatrixF * position;

			glNormal3f(mesh->normals[face->normals[j]][0], mesh->normals[face->normals[j]][1], mesh->normals[face->normals[j]][2]);
			glTexCoord2f(mesh->uvs[face->uvs[j]][0], mesh->uvs[face->uvs[j]][1]);
			glVertex3f(mesh->vertices[face->vertices[j]][0], mesh->vertices[face->vertices[j]][1], mesh->vertices[face->vertices[j]][2]);
		}
		glEnd();
	}

	++graphicsState.renderedObjects;		// increment rendered objects for debug info

	// Render children if needed
	/*
	if (child)
		for (int i = 0; i < children; ++i)
			child[i]->RenderOld(graphicsState);
*/

	// Revert the model matrix to the old one in the stack
	graphicsState.modelMatrixD = tmp;
}

/// Gets velocity, probably from the PhysicsState
Vector3f Entity::Velocity()
{
	if (physics)
		return physics->velocity;
	return Vector3f();
}

/// Mostly just checking graphics->visibility and if registered for rendering.
bool Entity::IsVisible()
{
	if (!registeredForRendering)
		return false;
	if (!graphics)
		return false;
	return graphics->visible;
}



/// Sets position
void Entity::SetPosition(ConstVec3fr position)
{
	this->localPosition = position;
	RecalculateMatrix();
}
/// Sets position
void Entity::SetPosition(float x, float y, float z)
{
	this->localPosition = Vector3f(x,y,z);
	RecalculateMatrix();
}

/// Rotates around the globally defined quaternion axis.
void Entity::RotateGlobal(const Quaternion & withQuaternion)
{
	// http://www.cprogramming.com/tutorial/3d/quaternions.html
	// Make sure you got it registerd for physics first... slightly retarded, but yeah.
	// Should maybe move the orientation quaternion out to the main entity class..?
	assert(physics);
	if (physics)
	{
		Quaternion result1, result2;
		result1 = physics->orientation * withQuaternion;
		result2 = withQuaternion * physics->orientation;
		physics->orientation = result1;
		physics->orientation.Normalize();
		hasRotated = true;
	}
	RecalculateMatrix();
}
	
/// Rotates the Entity
void Entity::Rotate(ConstVec3fr rotation)
{
	this->rotation += rotation;
	if (physics && physics->useQuaternions)
	{
		Quaternion rotationQuaternion = Quaternion(rotation, 1.0f);
		rotationQuaternion.Normalize();

		Quaternion result1, result2;
		result1 = physics->orientation * rotationQuaternion;
		result2 = rotationQuaternion * physics->orientation;
		physics->orientation = result1;
		physics->orientation.Normalize();
		hasRotated = true;
	}
	RecalculateMatrix();
}

/// Quaternion initial rotation.
void Entity::SetRotation(const Quaternion & quat)
{
	// http://www.cprogramming.com/tutorial/3d/quaternions.html
	// Make sure you got it registerd for physics first... slightly retarded, but yeah.
	// Should maybe move the orientation quaternion out to the main entity class..?
	assert(physics);
	if (physics)
	{
		physics->orientation = Quaternion();
		Quaternion result1, result2;
		result1 = physics->orientation * quat;
		result2 = quat * physics->orientation;
		physics->orientation = result1;
		physics->orientation.Normalize();
		hasRotated = true;
	}
	RecalculateMatrix();
}
	

void Entity::SetRotation(ConstVec3fr rotation)
{
//	assert(false);
	this->rotation = rotation;
	hasRotated = true;
	if (physics && physics->useQuaternions)
	{
		/// This assumes Euler angles, so construct an euler angle now!
		Quaternion pitch(Vector3f(1,0,0), -rotation.x), 
			yaw(Vector3f(0,1,0), rotation.y),
			roll(Vector3f(0,0,1), rotation.z);
		
		Quaternion pitchYaw = pitch * yaw;
		pitchYaw.Normalize();
		
		Quaternion newOrientation = pitch * yaw * roll;
		physics->orientation = newOrientation;
		physics->orientation.Normalize();

		//physics->orientation = Quaternion();
		//Quaternion rotationQuaternion = Quaternion(rotation, 1.0f);
		//rotationQuaternion.Normalize();
		//physics->orientation = physics->orientation * rotationQuaternion;
		//physics->orientation.Normalize();
	}
	RecalculateMatrix();
}

/// Scales all axes to target value.
void Entity::SetScale(float scale)
{
	SetScale(Vector3f(scale, scale, scale));
}

/// Sets scale of the entity
void Entity::SetScale(ConstVec3fr scale)
{
//	assert(scale[0] && scale[1] && scale[2]);
	this->scale = scale;
	UPDATE_SCALING_MATRIX
	RecalculateMatrix();
}
/// Scales the Entity
void Entity::Scale(ConstVec3fr scalerVec)
{
	this->scale *= scalerVec;
	UPDATE_SCALING_MATRIX
	RecalculateMatrix();
}
/// Scales the Entity
void Entity::Scale(float scaleF)
{
	this->scale *= scaleF;
	UPDATE_SCALING_MATRIX
	RecalculateMatrix();
}
/// Translates the Entity
void Entity::Translate(float x, float y, float z)
{
	assert(false && "old");/*
	this->position[0] += x;
	this->position[1] += y;
	this->position[2] += z;
	RecalculateMatrix();*/
}
/// Translates the Entity
void Entity::Translate(ConstVec3fr translation)
{
	this->localPosition += translation;
	RecalculateMatrix();
}

/// Recalculates the transformation matrix. All parts by default. If recursively, will update children (or parents?) recursively upon update.
void Entity::RecalculateMatrix(int whichParts/*= true*/, bool recursively /* = false*/)
{
	if (whichParts > TRANSLATION_ONLY || hasRotated)
	{
		if (whichParts == ALL_PARTS || hasRotated)
		{
			RecalcRotationMatrix(true);
		}
		localTransform = Matrix4f();
#ifdef USE_SSE
		// Translation should be just pasting in position.. no?
		localTransform.col3.data = localPosition.data;
		localTransform.col3.w = 1;
#else
		localTransform.Multiply((Matrix4f::Translation(position)));
#endif
		localTransform.Multiply(localRotation);
		// No use multiplying if not new scale.
		if (hasRescaled || whichParts)
			UPDATE_SCALING_MATRIX
		if (relevantScale)
			localTransform.Multiply(scalingMatrix);
			// Ensure it has a scale..?
	//	assert(transformationMatrix.HasValidScale());
	}
	// No rotation? -> 
	else 
	{
		// Just update position.
#ifdef USE_SSE
		localTransform.col3.data = localPosition.data;
		localTransform.col3.w = 1;
#else
		localTransform[12] = position[0];
		localTransform[13] = position[1];
		localTransform[14] = position[2];
#endif		
		// Children?
	}
	/// Use parent matrix, apply ours on top of it!
	if (parent)
	{
		if (inheritPositionOnly)
		{
			transformationMatrix = Matrix4f::Translation(parent->worldPosition) * localTransform;
		}
		else 
		{
			/// Transforms as calculated if this were not child of any other entity.
			transformationMatrix = parent->transformationMatrix * localTransform;
		}
	}
	else 
	{
		transformationMatrix = localTransform;
	}
	// Since we updated something, we should inform our children as well, if any, or they will be lagging behind...
	if (recursively && children.Size())
	{
		for (int i = 0; i < children.Size(); ++i)
		{
			children[i]->RecalculateMatrix(ALL_PARTS, true);
		}
	}
	worldPosition = transformationMatrix.Product(Vector4f());

	/// Update AABB accordingly.
	aabb->Recalculate(this);
}

void Entity::RecalcRotationMatrix(bool force /* = false*/)
{
#define EXTRACT_VECTORS \
	lookAt = rotationMatrix * Vector4f(0,0,-1,0);\
	upVec = rotationMatrix * Vector4f(0,1,0,0);\
	rightVec = rotationMatrix * Vector4f(1,0,0,0);
	/// Recalc normal matrix (rotation for all normals) based on the rotation matrix.
#define UPDATE_NORMAL_MATRIX	\
	normalMatrix = rotationMatrix.InvertedCopy().TransposedCopy();

	// If no rotation has occured, just multiply in the parent matrix?
	if (!hasRotated && !force)
	{
		if (parent)
		{
			rotationMatrix = parent->rotationMatrix * localRotation;
//			EXTRACT_VECTORS;
			UPDATE_NORMAL_MATRIX;
		}
		// No change? Then vectors should be the same as well.
		EXTRACT_VECTORS
		return;
	}
	localRotation = Matrix4d();
	// Quaternions for those entities wanting to use it.
	if (physics && physics->useQuaternions)
	{
		Quaternion q = physics->orientation;
	 //   std::cout<<"\nQ preN: "<<q;
		q.Normalize();
	 //   std::cout<<"\nQ postN: "<<q;
		localRotation = q.Matrix();
	   // float * parr = rotationMatrix.getPointer();
	 //   std::cout<<"\nMatrix: "<<parr[0]<<" "<<parr[1];

		Quaternion q2 = physics->preTranslateRotationQ;
		if (physics->preTranslateRotationQ.y != 0)
		{
			int i = int(+q2.y);
		}
		preTranslateMat = q2.Matrix();
	}
	// Euclidean co-ordinates.
	else 
	{
		localRotation.Multiply(Matrix4d::GetRotationMatrixX(rotation[0]));
		localRotation.Multiply(Matrix4d::GetRotationMatrixZ(rotation[2]));
		localRotation.Multiply(Matrix4d::GetRotationMatrixY(rotation[1]));
	}	

	if (parent)
	{
		rotationMatrix = parent->rotationMatrix * localRotation;
	}
	else
	{
		rotationMatrix = localRotation;
	}

	EXTRACT_VECTORS;
	UPDATE_NORMAL_MATRIX;

	hasRotated = false;
}


/// Recalculates the matrix this entity should have, using given position.
void Entity::RecalculateMatrix(Matrix4f & matrix, Vector3f * givenPosition)
{
	Matrix4f localTransform;
	if (true)
	{	
		// Only recalc rot if it has rotated, this is usually already recalcualted since the phyusics frame.
		if (hasRotated)
		{
			RecalcRotationMatrix(true);
		}
		localTransform = Matrix4f();
#ifdef USE_SSE
		// Translation should be just pasting in position.. no?
		localTransform.col3.data = givenPosition->data;
		localTransform.col3.w = 1;
#else
		localTransform.Multiply((Matrix4f::Translation(position)));
#endif
		localTransform.Multiply(localRotation);
		// No use multiplying if not new scale.
		if (hasRescaled)
			UPDATE_SCALING_MATRIX
		if (relevantScale)
			localTransform.Multiply(scalingMatrix);
			// Ensure it has a scale..?
	//	assert(localTransform.HasValidScale());
	}
	// No rotation? -> 
	else 
	{
		// Just update position.
#ifdef USE_SSE
		matrix.col3.data = givenPosition->data;
		matrix.col3.w = 1;
#else
		matrix[12] = position[0];
		matrix[13] = position[1];
		matrix[14] = position[2];
#endif		
		// Children?
	}
	/// Use parent matrix, apply ours on top of it!
	if (parent)
	{
		/// Transforms as calculated if this were not child of any other entity.
		matrix = parent->transformationMatrix * localTransform;
	}
	else 
	{
		matrix = localTransform;
	}
	// Don't recalculate for children, since this is just for graphics smoothing...(?)
	/*
	// Since we updated something, we should inform our children as well, if any, or they will be lagging behind...
	if (recursively && children.Size())
	{
		for (int i = 0; i < children.Size(); ++i)
		{
			children[i]->RecalculateMatrix(ALL_PARTS, true);
		}
	}*/
	// Don't extract anything here. Just used for rendering.
//	worldPosition = transformationMatrix.Product(Vector4f());

	/// Update AABB accordingly.
//	aabb->Recalculate(this);
}

/*
void Entity::RecalculateMatrix(Matrix4f & givenMatrix, Vector3f * position)
{
}*/

/// Recalculates a transformation matrix using argument vectors for position, rotation and translation.
Matrix4f Entity::RecalculateMatrix(ConstVec3fr position, ConstVec3fr rotation, ConstVec3fr scale)
{
	assert(false && "Will be replaced?");
	/*
	Matrix4d rotationMatrix;		

	rotationMatrix.Multiply(Matrix4d::GetRotationMatrixX(rotation.x));
	rotationMatrix.Multiply(Matrix4d::GetRotationMatrixY(rotation.y));
	rotationMatrix.Multiply(Matrix4d::GetRotationMatrixZ(rotation.z));
	Matrix4d matrix = Matrix4d();

	matrix.Multiply((Matrix4d().Translate(Vector3d(position))));
	matrix.Multiply(rotationMatrix);
	matrix.Multiply((Matrix4d().Scale(Vector3d(scale))));

	// Ensure it has a scale..?
	assert(matrix.HasValidScale());
	return matrix;
	*/
	return Matrix4f();
}

/// Recalculates the radius of the entity, both in the upper level radius as well as the physics-property variable one if applicable.
void Entity::RecalculateRadius()
{
	float newRadius = model->Radius() * scale.MaxPart();
	/// Recalculate physical radius too.
	if (physics && physics->recalculatePhysicalRadius)
		physics->physicalRadius = newRadius;
}

/// Sets name for this entity.
bool Entity::SetName(const char * i_name){
	name = i_name;
	return true;
};

/// Sets model to be used by this entity.
bool Entity::SetModel(Model * i_model)
{
	if (!i_model)
		return false;
	if (model)
		--model->users;
	model = i_model;
	if (model)
	{
		++model->users;
	}
	return true;
};

/// Sets texture to be used by this entity
bool Entity::SetTexture(int target, Texture * i_texture)
{
	if (target & DIFFUSE_MAP){
		if (diffuseMap)
			--diffuseMap->users;
		diffuseMap = i_texture;
		if (diffuseMap)
			++diffuseMap->users;
	}
	if (target & SPECULAR_MAP){
		if (specularMap)
			--specularMap->users;
		specularMap = i_texture;
		if (specularMap)
			++specularMap->users;
	}
	if (target & NORMAL_MAP){
		if (normalMap)
			--normalMap->users;
		normalMap = i_texture;
		if (normalMap)
			++normalMap->users;
	}
	if (target & EMISSIVE_MAP)
	{
		if (emissiveMap)
			--emissiveMap->users;
		emissiveMap = i_texture;
		if (emissiveMap)
			++emissiveMap->users;
	}
	return true;
}

/// Returns current texture bound to the target.
Texture * Entity::GetTexture(int target){
	switch(target){
		case DIFFUSE_MAP:
			return diffuseMap;
		case SPECULAR_MAP:
			return specularMap;
		case NORMAL_MAP:
			return normalMap;
	}
	return NULL;
}

List<Texture*> Entity::GetTextures(int targetFlags)
{
	List<Texture*> texs;
	if (targetFlags & DIFFUSE_MAP)
		texs.Add(diffuseMap);
	if (targetFlags & SPECULAR_MAP)
		texs.Add(specularMap);
	if (targetFlags & NORMAL_MAP)
		texs.Add(normalMap);
	if (targetFlags & EMISSIVE_MAP)
		texs.Add(emissiveMap);
	return texs;
}


/// Returns path for current texture's source.
String Entity::GetTextureSource(int target)
{
	Texture * tex = GetTexture(target);
	if (!tex)
		return String();
	return tex->source;
}

/// Returns all faces of the entity, transformed with it's current transformation. COSTLY FUNCTION.
List<Triangle> Entity::GetTris()
{
	List<Triangle> triangles;
	if (model){
		triangles += model->GetTris();
	}
	for (int i = 0; i < triangles.Size(); ++i){
		Triangle & tri = triangles[i];
		Vector3f prevPos = triangles[i].position;
		tri.Transform(transformationMatrix);
		Vector3f newPos = triangles[i].position;
		/// Seems to be working! :)
//		if (position.LengthSquared() > 1.0f || scale.LengthSquared() > 1.0f)
			;//assert(prevPos[0] != newPos[0]);
	}
	return triangles;
}


/// Returns the center of this entity, determined by position, rotation, and current model.
Vector3f Entity::CenterOfGravityWorldSpace()
{
	// Fetch the model or something.
	if (!model)
		return worldPosition;
	Vector4f center = model->centerOfModel;
	// Multiply co-ordinates of model center with our matrix.
	Vector4f centerWorldSpace = transformationMatrix.Product(center);
	return centerWorldSpace;
}

/// Checks with Rotation matrix.
Vector3f Entity::LookAt()
{
	return lookAt;
}

Vector3f Entity::UpVec()
{
	return upVec;
}
Vector3f Entity::RightVec()
{
	return rightVec;
}

/// Radius of the bounding sphere.
float Entity::Radius() const
{
	float rad = 0.f;
	if (model)
		rad = model->Radius();
	return rad * this->scale.MaxPart();
}


/// o.o Links child and parent for both.
void Entity::AddChild(Entity * child)
{
	assert(child->parent == NULL);
	children.Add(child);
	child->parent = this;
}
