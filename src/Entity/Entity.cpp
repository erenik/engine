// Emil Hedemalm
// 2013-03-17

#include "Mesh/Mesh.h"

#include "../Material.h"
#include "Model/Model.h"
#include "Entity.h"
#include "Graphics/GraphicsProperty.h"
#include "Graphics/CompactGraphics.h"

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

const Material Entity::defaultMaterial = Material();

/// Creates a compact entity out of this Entity object
void Entity::CreateCompactEntity(CompactEntity * cEntity){
	assert(cEntity);
	strcpy(cEntity->name, name.c_str());
	strcpy(cEntity->model, model->RelativePath().c_str());
	if (diffuseMap)
		strcpy(cEntity->diffuseMap, diffuseMap->RelativePath().c_str());
	if (specularMap)
		strcpy(cEntity->specularMap, specularMap->RelativePath().c_str());
	if (normalMap)
		strcpy(cEntity->normalMap, normalMap->RelativePath().c_str());
	cEntity->position = position;
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
	position = cEntity->position;
	scale = cEntity->scale;
	rotation = cEntity->rotation;
	if (cEntity->cPhysics){
		assert(!physics);
		physics = new PhysicsProperty(cEntity->cPhysics);
		Physics.QueueMessage(new PMSetPhysicsType(this, physics->type));
		Physics.QueueMessage(new PMSetPhysicsShape(this, physics->physicsShape));
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
	updateChildrenOnTransform = false;
	position = Vector3f(0,0,0);
	scale = Vector3f(1,1,1);
	rotation = Vector3f(0,0,0);
	flags = 0;
	radius = 1;
	diffuseMap = NULL;
	specularMap = NULL;
	normalMap = NULL;
	material = new Material(defaultMaterial);
	model = NULL;
	id = i_id;
	/// Owner o-o
	this->player = NULL;

	aabb = NULL;

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
}

/// Default constructor...
Entity::~Entity()
{
	/// Delete safe stuff.
	Delete();
	/// Delete those things that should have been deleted elsewhere too.
#define SAFE_DELETE(p) {if(p) delete p; p = NULL; }
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

/// If reacting to collisions,.. pokes all properties about it too.
void Entity::OnCollision(Collision & data)
{
	for (int i = 0; i < properties.Size(); ++i)
	{
		EntityProperty * prop = properties[i];
		prop->OnCollision(data);
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



/** Buffers this entities' models into graphics memory.
	Should only be used by the graphics manager. USE WITH CAUTION.
*/
void Entity::Bufferize()
{
	if (model)
		model->GetTriangulizedMesh()->Bufferize(false, false);
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
	else if (diffuseMap == NULL){
		glBindTexture(GL_TEXTURE_2D, NULL);
		graphicsState.currentTexture = NULL;
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
	this->position = position;
	RecalculateMatrix();
}
/// Sets position
void Entity::SetPosition(float x, float y, float z)
{
	this->position = Vector3f(x,y,z);
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
	}
	RecalculateMatrix();
}
	

void Entity::SetRotation(ConstVec3fr rotation)
{
//	assert(false);
	this->rotation = rotation;
	if (physics && physics->useQuaternions)
	{
		/// This assumes Euler angles, so construct an euler angle now!
		Quaternion pitch(Vector3f(1,0,0), rotation[0]), 
			yaw(Vector3f(0,1,0), rotation[1]),
			roll(Vector3f(0,0,1), rotation[2]);
		
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

/// Sets scale of the entity
void Entity::SetScale(ConstVec3fr scale)
{
//	assert(scale[0] && scale[1] && scale[2]);
	this->scale = scale;
	RecalculateMatrix();
}
/// Scales the Entity
void Entity::Scale(ConstVec3fr scale){
	this->scale = Vector3f(this->scale[0] * scale[0], this->scale[1] * scale[1], this->scale[2] * scale[2]);
	RecalculateMatrix();
}
/// Scales the Entity
void Entity::Scale(float scale){
	this->scale = Vector3f(this->scale[0] * scale, this->scale[1] * scale, this->scale[2] * scale);
	RecalculateMatrix();
}
/// Translates the Entity
void Entity::Translate(float x, float y, float z){
	this->position[0] += x;
	this->position[1] += y;
	this->position[2] += z;
	RecalculateMatrix();
}
/// Translates the Entity
void Entity::Translate(ConstVec3fr translation)
{
	this->position =  this->position + translation;
	RecalculateMatrix();
}

/// Recalculates the transformation matrix. All parts by default. If recursively, will update children (or parents?) recursively upon update.
void Entity::RecalculateMatrix(bool allParts /*= true*/, bool recursively /* = false*/)
{
    Matrix4f preTranslateMat;
	if (allParts || hasRotated)
	{
		rotationMatrix = Matrix4d();
		// Quaternions for those entities wanting to use it.
		if (physics && physics->useQuaternions){
			Quaternion q = physics->orientation;
		 //   std::cout<<"\nQ preN: "<<q;
			q.Normalize();
		 //   std::cout<<"\nQ postN: "<<q;
			rotationMatrix = q.Matrix();
		   // float * parr = rotationMatrix.getPointer();
		 //   std::cout<<"\nMatrix: "<<parr[0]<<" "<<parr[1];

			Quaternion q2 = physics->preTranslateRotationQ;
			if (physics->preTranslateRotationQ.y != 0)
			{
				int i = + q2.y;
			}
			preTranslateMat = q2.Matrix();
		}
		// Euclidean co-ordinates.
		else 
		{
			rotationMatrix.Multiply(Matrix4d::GetRotationMatrixX(rotation[0]));
			rotationMatrix.Multiply(Matrix4d::GetRotationMatrixZ(rotation[2]));
			rotationMatrix.Multiply(Matrix4d::GetRotationMatrixY(rotation[1]));
		}	
		hasRotated = false;
		

		transformationMatrix = Matrix4f();

		transformationMatrix.Multiply(preTranslateMat);
		transformationMatrix.Multiply((Matrix4f::Translation(position)));
		transformationMatrix.Multiply(rotationMatrix);
		transformationMatrix.Multiply((Matrix4f::Scaling(scale)));

		/// Use parent matrix, apply ours on top of it!
		if (parent)
		{
			transformationMatrix = parent->transformationMatrix * transformationMatrix;
		}

		// Since we updated something, we should inform our children as well, if any, or they will be lagging behind...
		if (updateChildrenOnTransform || recursively)
		{
			for (int i = 0; i < children.Size(); ++i)
			{
				children[i]->RecalculateMatrix(true, true);
			}
		}

		worldPosition = transformationMatrix.Product(Vector4f());
			// Ensure it has a scale..?
	//	assert(transformationMatrix.HasValidScale());
	}
	// No rotation? -> 
	else 
	{
		// Just update position.
		transformationMatrix[12] = position[0];
		transformationMatrix[13] = position[1];
		transformationMatrix[14] = position[2];
	}
	worldPosition = transformationMatrix * Vector3f();
}

/// Recalculates a transformation matrix using argument vectors for position, rotation and translation.
Matrix4f Entity::RecalculateMatrix(ConstVec3fr position, ConstVec3fr rotation, ConstVec3fr scale)
{
	Matrix4d rotationMatrix;		

	rotationMatrix.Multiply(Matrix4d::GetRotationMatrixX(rotation[0]));
	rotationMatrix.Multiply(Matrix4d::GetRotationMatrixY(rotation[1]));
	rotationMatrix.Multiply(Matrix4d::GetRotationMatrixZ(rotation[2]));
	Matrix4d matrix = Matrix4d();

	matrix.Multiply((Matrix4d().Translate(Vector3d(position))));
	matrix.Multiply(rotationMatrix);
	matrix.Multiply((Matrix4d().Scale(Vector3d(scale))));

	// Ensure it has a scale..?
	assert(matrix.HasValidScale());
	return matrix;
}

/// Recalculates the radius of the entity, both in the upper level radius as well as the physics-property variable one if applicable.
void Entity::RecalculateRadius()
{
	float newRadius = model->radius * scale.MaxPart();
	this->radius = newRadius;
	/// Recalculate physical radius too.
	if (physics)
		physics->physicalRadius = radius;
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
		radius = model->radius;
	}
	// No model?
	else 
	{
		// No radius.
		radius = 0;
	}
	return true;
};

/// Sets texture to be used by this entity
bool Entity::SetTexture(int target, Texture * i_texture){
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
		if (position.LengthSquared() > 1.0f || scale.LengthSquared() > 1.0f)
			;//assert(prevPos[0] != newPos[0]);
	}
	return triangles;
}


/// Returns the center of this entity, determined by position, rotation, and current model.
Vector3f Entity::CenterOfGravityWorldSpace()
{
	// Fetch the model or something.
	if (!model)
		return position;
	Vector4f center = model->centerOfModel;
	// Multiply co-ordinates of model center with our matrix.
	Vector4f centerWorldSpace = transformationMatrix.Product(center);
	return centerWorldSpace;
}

/// Checks with Rotation matrix.
Vector3f Entity::LookAt()
{
	return rotationMatrix * Vector3f(0,0,-1);
}

/// o.o Links child and parent for both.
void Entity::AddChild(Entity * child)
{
	assert(child->parent == NULL);
	children.Add(child);
	child->parent = this;
}
