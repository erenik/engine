// Emil Hedemalm
// 2013-03-17

#include "../Material.h"
#include "../Model.h"
#include "Entity.h"
#include "Graphics/GraphicsProperty.h"
#include "Graphics/CompactGraphics.h"
#include "Physics/PhysicsManager.h"
#include "Physics/CompactPhysics.h"
#include "Physics/PhysicsProperty.h"
#include "Pathfinding/PathfindingProperty.h"
#include "EntityStates/StateProperty.h"
#include "CompactEntity.h"
#include "../GraphicsState.h"
#include "Texture.h"
#include "Shader.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Event/EventProperty.h"
#include <cstring>

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
	cEntity->position = positionVector;
	cEntity->scale = scaleVector;
	cEntity->rotation = rotationVector;
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
void Entity::LoadCompactEntityData(CompactEntity * cEntity){
	/// Model and texture is extracted elsewhere, but we can copy name, and other details here..
	name = cEntity->name;
	positionVector = cEntity->position;
	scaleVector = cEntity->scale;
	rotationVector = cEntity->rotation;
	if (cEntity->cPhysics){
		assert(!physics);
		physics = new PhysicsProperty(cEntity->cPhysics);
		Physics.QueueMessage(new PMSetPhysicsType(this, physics->type));
		Physics.QueueMessage(new PMSetPhysicsShape(this, physics->physicsShape));
	}
	if (cEntity->cGraphics){
		assert(!graphics);
		graphics = new GraphicsProperty();
		graphics->LoadDataFrom(cEntity->cGraphics);
	}
	diffuseMap = TexMan.GetTextureBySource(cEntity->diffuseMap);
	specularMap = TexMan.GetTextureBySource(cEntity->specularMap);
	normalMap = TexMan.GetTextureBySource(cEntity->normalMap);
	model = ModelMan.GetModel(cEntity->model);
	/// Recalculate model matrix after position update :P
	recalculateMatrix();
}

Entity::Entity(int i_id){
	positionVector = Vector3f(0,0,0);
	scaleVector = Vector3f(1,1,1);
	rotationVector = Vector3f(0,0,0);
	flags = 0;
	radius = 1;
	diffuseMap = NULL;
	specularMap = NULL;
	normalMap = NULL;
	material = new Material(defaultMaterial);
	model = NULL;
	child = NULL;
	children = 0;
	id = i_id;
	/// Owner o-o
	this->player = NULL;

	// Default all flags to 0.
	/// Status, for whether it's part of rendering, physics, etc.
	this->registeredForRendering = false;
	this->registeredForPhysics = false;
	this->flaggedForDeletion = false;

	/// Nullify all pointers to add-ons. IMPORTANT!
	this->graphics = NULL;
	this->physics = NULL;
	this->state = NULL;
	this->events = NULL;
	/// Create it automatiaclly so we don't have to, cheers..
	this->pathfindingProperty = new PathfindingProperty(this);
}

/// Default constructor...
Entity::~Entity(){
	/// Delete safe stuff.
	Delete();
	/// Delete those things that should have been deleted elsewhere too.
#define SAFE_DELETE(p) {if(p) delete p; p = NULL; }
	SAFE_DELETE(graphics);
	SAFE_DELETE(physics);
	SAFE_DELETE(events);
	SAFE_DELETE(pathfindingProperty);
	if (material)
		delete material;
	material = NULL;
	// Models and textures will be deallocated by their respectice managers!
}

/// Deallocates additional points as needed.
void Entity::Delete()
{
	SAFE_DELETE(state);
}

/** Buffers this entities' models into graphics memory.
	Should only be used by the graphics manager. USE WITH CAUTION.
*/
void Entity::Bufferize(){
	this->model->triangulizedMesh->Bufferize();
}

void Entity::RenderOld(GraphicsState &graphicsState){
	int error = 0;
	// Bind texture if it isn't already bound.
	if (diffuseMap && graphicsState.currentTexture != diffuseMap){
		// When rendering an objectwith this program.
		glActiveTexture(GL_TEXTURE0 + 0);
		// Bind texture
		glBindTexture(GL_TEXTURE_2D, diffuseMap->glid);
		error = glGetError();

		// Set sampler in client graphicsState
		if (graphicsState.activeShader && graphicsState.activeShader->uniformBaseTexture != -1)
			glUniform1i(graphicsState.activeShader->uniformBaseTexture, 0);		// Sets sampler
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
	if (graphicsState.activeShader){
		glUniform4f(graphicsState.activeShader->uniformMaterial.ambientVec4, material->ambient[0], material->ambient[1], material->ambient[2], material->ambient[3]);
		glUniform4f(graphicsState.activeShader->uniformMaterial.diffuseVec4, material->diffuse[0], material->diffuse[1], material->diffuse[2], material->diffuse[3]);
		glUniform4f(graphicsState.activeShader->uniformMaterial.specularVec4, material->specular[0], material->specular[1], material->specular[2], material->specular[3]);
		glUniform1i(graphicsState.activeShader->uniformMaterial.shininessInt, material->shininess);
	}

	error = glGetError();
	// Save old matrix to the stack
	Matrix4d tmp = graphicsState.modelMatrixD;

	// Apply transformation
	graphicsState.modelMatrixD.multiply(transformationMatrix);
	graphicsState.modelMatrixF = graphicsState.modelMatrixD;
	Matrix4f modelView = graphicsState.viewMatrixF * graphicsState.modelMatrixF;
	// Set uniform matrix in shader to point to the GameState modelView matrix.
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelView.getPointer());
	if (graphicsState.activeShader)
		glUniformMatrix4fv(graphicsState.activeShader->uniformModelMatrix, 1, false, graphicsState.modelMatrixF.getPointer());
	error = glGetError();

	// Set texture
//	if (texture && texture->glid){
//		glBindTexture(GL_TEXTURE_2D, texture->glid);
//	}

	// Render the model, old-school
	Mesh * mesh = model->triangulizedMesh;
	for (int i = 0; i < mesh->faces; ++i){
		glBegin(GL_TRIANGLES);
		MeshFace * face = &mesh->face[i];
		if (face->numVertices > 3){
			std::cout<<"\nmesh face with more than 3 vertices D:";
		}
		for (int j = 0; j < 3; ++j){
			Vector3f position = Vector3f(mesh->vertex[face->vertex[j]].x, mesh->vertex[face->vertex[j]].y, mesh->vertex[face->vertex[j]].z);
			position = graphicsState.modelMatrixF * position;
			position = graphicsState.viewMatrixF * position;
			position = graphicsState.projectionMatrixF * position;

			glNormal3f(mesh->normal[face->normal[j]].x, mesh->normal[face->normal[j]].y, mesh->normal[face->normal[j]].z);
			glTexCoord2f(mesh->u[face->uv[j]],mesh->v[face->uv[j]]);
			glVertex3f(mesh->vertex[face->vertex[j]].x, mesh->vertex[face->vertex[j]].y, mesh->vertex[face->vertex[j]].z);
		}
		glEnd();
	}

	++graphicsState.renderedObjects;		// increment rendered objects for debug info

	// Render children if needed
	if (child)
		for (int i = 0; i < children; ++i)
			child[i]->RenderOld(graphicsState);

	// Revert the model matrix to the old one in the stack
	graphicsState.modelMatrixD = tmp;
}

/// Rendering method
void Entity::render(GraphicsState &graphicsState){
	if (graphicsState.settings & USE_LEGACY_GL){
		RenderOld(graphicsState);
		return;
	}
	int error = 0;

	// To send to the shadar
	int texturesToApply = 0;

	// Find diffuseMap!
	if (this->graphics && this->graphics->hasAnimation){

		diffuseMap = this->graphics->GetTextureForCurrentFrame(graphicsState.currentFrameTime);
	//	diffuseMap = TexMan.GetTextureByName("Checkpoints/test");
	//	diffuseMap = TexMan.GetTextureByName("RuneRPG/Units/200");
	//	diffuseMap = TexMan.GetTextureByName("logo8");
		if (diffuseMap->glid == -1){
			/// Bufferize it
			TexMan.BufferizeTexture(diffuseMap);
		}
	}

	// Bind texture if it isn't already bound.
	if (diffuseMap && graphicsState.currentTexture == diffuseMap){
		texturesToApply |= DIFFUSE_MAP;
	}
	else if (diffuseMap && graphicsState.currentTexture != diffuseMap){
		texturesToApply |= DIFFUSE_MAP;
		// When rendering an object with this program.
		glActiveTexture(GL_TEXTURE0 + 0);		// Select server-side active texture unit

		// Bind texture
		glBindTexture(GL_TEXTURE_2D, diffuseMap->glid);
		error = glGetError();

		// Set sampler in client graphicsState
		if (graphicsState.activeShader->uniformBaseTexture != -1)
			glUniform1i(graphicsState.activeShader->uniformBaseTexture, 0);		// Sets sampler
		error = glGetError();
	// Assign material
		// Bind sampler
		/// Core since 3.3!
		// glBindSampler(0, StateMan.sampler[0]);				// Sets sampler graphicsState
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		graphicsState.currentTexture = diffuseMap;
	}
	else if (diffuseMap == NULL){
		glActiveTexture(GL_TEXTURE0 + 0);		// Select server-side active texture unit
		glBindTexture(GL_TEXTURE_2D, NULL);
		graphicsState.currentTexture = NULL;
	}

	/// Bind specular if it isn't already o-o
	if (specularMap){
		texturesToApply |= SPECULAR_MAP;
		glActiveTexture(GL_TEXTURE0 + 1);		// Select server-side active texture unit

		// Bind texture
		glBindTexture(GL_TEXTURE_2D, specularMap->glid);
		error = glGetError();

		// Set sampler in client graphicsState
		if (graphicsState.activeShader->uniformSpecularMap!= -1)
			glUniform1i(graphicsState.activeShader->uniformSpecularMap, 1);		// Sets sampler
		error = glGetError();
	// Assign material
		// Bind sampler
		/// Core since 3.3!
		// glBindSampler(0, StateMan.sampler[0]);				// Sets sampler graphicsState
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		graphicsState.currentSpecularMap = specularMap;
		glActiveTexture(GL_TEXTURE0 + 0);
	}
	else {
		glActiveTexture(GL_TEXTURE0 + 1);		// Select server-side active texture unit
		glBindTexture(GL_TEXTURE_2D, NULL);
		glActiveTexture(GL_TEXTURE0 + 0);
	//	glBindTexture(GL_TEXTURE_2D, NULL);
	}

	// Bind normalMap too if it isn't already!
/*	if (normalMap == NULL){
		normalMap = TexMan.GetTextureBySource"normalMapTest2");
	}
*/	if (normalMap){
		texturesToApply |= NORMAL_MAP;
		glActiveTexture(GL_TEXTURE0 + 2);		// Select server-side active texture unit

		// Bind texture
		glBindTexture(GL_TEXTURE_2D, normalMap->glid);
		error = glGetError();

		// Set sampler in client graphicsState
		if (graphicsState.activeShader->uniformNormalMap != -1)
			glUniform1i(graphicsState.activeShader->uniformNormalMap, 2);		// Sets sampler
		error = glGetError();
	// Assign material
		// Bind sampler
		/// Core since 3.3!
		// glBindSampler(0, StateMan.sampler[0]);				// Sets sampler graphicsState
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		graphicsState.currentNormalMap = normalMap;
		glActiveTexture(GL_TEXTURE0 + 0);

	}
	else {
		glActiveTexture(GL_TEXTURE0 + 2);		// Select server-side active texture unit
		glBindTexture(GL_TEXTURE_2D, NULL);
		glActiveTexture(GL_TEXTURE0 + 0);
	//	glBindTexture(GL_TEXTURE_2D, NULL);
	}


	if (texturesToApply & DIFFUSE_MAP)
		glUniform1i(graphicsState.activeShader->uniformUseDiffuseMap, 1);
	else
		glUniform1i(graphicsState.activeShader->uniformUseDiffuseMap, 0);

	if (texturesToApply & SPECULAR_MAP)
		glUniform1i(graphicsState.activeShader->uniformUseSpecularMap, 1);
	else
		glUniform1i(graphicsState.activeShader->uniformUseSpecularMap, 0);

	if (texturesToApply & NORMAL_MAP)
		glUniform1i(graphicsState.activeShader->uniformUseNormalMap, 1);
	else
		glUniform1i(graphicsState.activeShader->uniformUseNormalMap, 0);

	// Send in texture-data to shader!
	// Set uniform matrix in shader to point to the GameState modelView matrix.
/*	GLuint uniform = glGetUniformLocation(graphicsState.activeShader->shaderProgram, "texturesToApply");
	if (uniform != -1)
		glUniform1i(uniform, texturesToApply);
*/

	error = glGetError();
	// Assign material
	// glProgramUniform Core since version 	4.1!
	// Use glUniform! Core since 2.0
//	glUniform4f(graphicsState.activeShader->uniformMaterial.ambientVec4, material->ambient[0], material->ambient[1], material->ambient[2], material->ambient[3]);
//	glUniform4f(graphicsState.activeShader->uniformMaterial.diffuseVec4, material->diffuse[0], material->diffuse[1], material->diffuse[2], material->diffuse[3]);
//	glUniform4f(graphicsState.activeShader->uniformMaterial.specularVec4, material->specular[0], material->specular[1], material->specular[2], material->specular[3]);
//	glUniform1i(graphicsState.activeShader->uniformMaterial.shininessInt, material->shininess);

	error = glGetError();
	// Save old matrix to the stack
	Matrix4d tmp = graphicsState.modelMatrixD;
	// Apply transformation
	graphicsState.modelMatrixD.multiply(transformationMatrix);
	graphicsState.modelMatrixF = graphicsState.modelMatrixD;
	// Set uniform matrix in shader to point to the GameState modelView matrix.
	glUniformMatrix4fv(graphicsState.activeShader->uniformModelMatrix, 1, false, graphicsState.modelMatrixF.getPointer());
	error = glGetError();

	// Set uniform matrix in shader to point to the GameState modelView matrix.
	GLuint uniform = glGetUniformLocation(graphicsState.activeShader->shaderProgram, "normalMatrix");
	Vector3f normal = Vector3f(0,1,0);
	Matrix4f normalMatrix = graphicsState.modelMatrixF.InvertedCopy().TransposedCopy();
	normal = normalMatrix.product(normal);
	if (uniform != -1)
		glUniformMatrix4fv(uniform, 1, false, normalMatrix.getPointer());
	error = glGetError();

	// Set texture
//	if (texture && texture->glid){
//		glBindTexture(GL_TEXTURE_2D, texture->glid);
//	}

	bool requiresSorting = false;
	bool render = true;
//	std::cout<<"Rendererr";

	// Check for modifiers to apply
	if (graphics && graphicsState.settings & ENABLE_SPECIFIC_ENTITY_OPTIONS){
		if (graphics->flags & RenderFlags::DISABLE_DEPTH_WRITE)
			glDepthMask(GL_FALSE);
		if (graphics->flags & RenderFlags::DISABLE_BACKFACE_CULLING)
			glDisable(GL_CULL_FACE);
		if (graphics->flags & RenderFlags::REQUIRES_DEPTH_SORTING){
			bool renderSortedEntities = graphicsState.settings & RENDER_SORTED_ENTITIES;
			if (!renderSortedEntities)
				requiresSorting = true;
		}

	}
	/// If requries sorting, save it in ze list
	if (requiresSorting){
		graphicsState.entitiesRequiringSorting.Add(this);
		render = false;
	}
	// Only render if previous states say so.
	if (render){
		// Render the model
		model->triangulizedMesh->Render(graphicsState);
		++graphicsState.renderedObjects;		// increment rendered objects for debug info
	}
	// Disable any modifiers now, unless we got a modifier that tells us to apply the previous modifiers to the children as well.
	if (graphics && graphicsState.settings & ENABLE_SPECIFIC_ENTITY_OPTIONS){
		if (graphics->flags & RenderFlags::DISABLE_DEPTH_WRITE)
			glDepthMask(GL_TRUE);
		if (graphics->flags & RenderFlags::DISABLE_BACKFACE_CULLING)
			glEnable(GL_CULL_FACE);
	}

	// Render children if needed
	if (child)
		for (int i = 0; i < children; ++i)
			child[i]->render(graphicsState);

	// Revert the model matrix to the old one in the stack
	graphicsState.modelMatrixD = tmp;

	/// Stock up graphical effects to render if any
	if (graphics != NULL){
		if (graphics->effects != NULL)
			graphicsState.graphicEffectsToBeRendered += *graphics->effects;
        if (graphics->particleSystems != NULL)
            graphicsState.particleEffectsToBeRendered += *graphics->particleSystems;
	}
}

/// Gets velocity, probably from the PhysicsState
Vector3f Entity::Velocity()
{
	if (physics)
		return physics->velocity;
	return Vector3f();
}


/// Sets position
void Entity::position(Vector3f position){
	this->positionVector = position;
	recalculateMatrix();
}
/// Sets position
void Entity::position(float x, float y, float z){
	this->positionVector = Vector3f(x,y,z);
	recalculateMatrix();
}
/// Rotates the Entity
void Entity::rotate(Vector3f rotation){
	/// Deprecateeeed.
//	assert(false);
	this->rotationVector += rotation;
#ifdef USE_QUATERNIONS
	assert(this->physics);
	this->physics->orientation *= Quaternion(rotation, 1.0f);
#endif
	recalculateMatrix();
}

/// Sets scale of the entity
void Entity::SetScale(Vector3f scale){
	this->scaleVector = scale;
	recalculateMatrix();
}
/// Scales the Entity
void Entity::Scale(Vector3f scale){
	this->scaleVector = Vector3f(this->scaleVector.x * scale.x, this->scaleVector.y * scale.y, this->scaleVector.z * scale.z);
	recalculateMatrix();
}
/// Scales the Entity
void Entity::Scale(float scale){
	this->scaleVector = Vector3f(this->scaleVector.x * scale, this->scaleVector.y * scale, this->scaleVector.z * scale);
	recalculateMatrix();
}
/// Translates the Entity
void Entity::translate(float x, float y, float z){
	this->positionVector.x += x;
	this->positionVector.y += y;
	this->positionVector.z += z;
	recalculateMatrix();
}
/// Translates the Entity
void Entity::translate(Vector3f translation){
	this->positionVector =  this->positionVector + translation;
	recalculateMatrix();
}

/// Recalculates the transformation matrix
void Entity::recalculateMatrix(){

    rotationMatrix = Matrix4d();

#ifdef USE_QUATERNIONS

    if (physics){
        Quaternion q = physics->orientation;
     //   std::cout<<"\nQ preN: "<<q;
        q.Normalize();
     //   std::cout<<"\nQ postN: "<<q;
        rotationMatrix = q.Matrix();
        double * parr = rotationMatrix.getPointer();
     //   std::cout<<"\nMatrix: "<<parr[0]<<" "<<parr[1];
    }
    else
        rotationMatrix = Matrix4d();
#else
	rotationMatrix.multiply(Matrix4d::GetRotationMatrixX(rotationVector.x));
	rotationMatrix.multiply(Matrix4d::GetRotationMatrixY(rotationVector.y));
	rotationMatrix.multiply(Matrix4d::GetRotationMatrixZ(rotationVector.z));
#endif

	transformationMatrix = Matrix4d();

	transformationMatrix.Multiply((Matrix4d().translate(Vector3d(positionVector))));
	transformationMatrix.Multiply(rotationMatrix);
	transformationMatrix.Multiply((Matrix4d().scale(Vector3d(scaleVector))));



}

/// Sets name for this entity.
bool Entity::SetName(const char * i_name){
	name = i_name;
	return true;
};

/// Sets model to be used by this entity.
bool Entity::SetModel(Model * i_model){
	if (!i_model)
		return false;
	if (model)
		--model->users;
	model = i_model;
	if (model)
		--model->users;
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
List<Triangle> Entity::GetTris(){
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
		if (positionVector.LengthSquared() > 1.0f || scaleVector.LengthSquared() > 1.0f)
			;//assert(prevPos.x != newPos.x);
	}
	return triangles;
}
