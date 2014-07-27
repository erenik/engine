// Emil Hedemalm
// 2013-03-17

#include "Mesh.h"

#include "../Material.h"
#include "../Model.h"
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
#include "ModelManager.h"
#include "Script/ScriptProperty.h"
#include "Physics/Calc/EntityPhysicsEstimator.h"
#include <cstring>
#include "Graphics/GraphicsManager.h"
#include "Graphics/Fonts/TextFont.h"
#include "EntityProperty.h"

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
	this->scripts = NULL;
	/// Create it automatiaclly so we don't have to, cheers..
	this->pathfindingProperty = new PathfindingProperty(this);
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


/** Buffers this entities' models into graphics memory.
	Should only be used by the graphics manager. USE WITH CAUTION.
*/
void Entity::Bufferize()
{
	if (model)
		model->GetTriangulizedMesh()->Bufferize();
}

void Entity::RenderOld(GraphicsState * graphicsState){
	int error = 0;
	// Bind texture if it isn't already bound.
	if (diffuseMap && graphicsState->currentTexture != diffuseMap){
		// When rendering an objectwith this program.
		glActiveTexture(GL_TEXTURE0 + 0);
		// Bind texture
		glBindTexture(GL_TEXTURE_2D, diffuseMap->glid);
		error = glGetError();

		// Set sampler in client graphicsState
		if (graphicsState->activeShader && graphicsState->activeShader->uniformBaseTexture != -1)
			glUniform1i(graphicsState->activeShader->uniformBaseTexture, 0);		// Sets sampler
		// Bind sampler
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		graphicsState->currentTexture = diffuseMap;
		glEnable(GL_TEXTURE_2D);
		glColor4f(1,1,1,1);
	}
	else if (diffuseMap == NULL){
		glBindTexture(GL_TEXTURE_2D, NULL);
		graphicsState->currentTexture = NULL;
	}

	error = glGetError();
	// Assign material
	// glProgramUniform Core since version 	4.1!
	// Use glUniform! Core since 2.0
	if (graphicsState->activeShader){
		glUniform4f(graphicsState->activeShader->uniformMaterial.ambientVec4, material->ambient[0], material->ambient[1], material->ambient[2], material->ambient[3]);
		glUniform4f(graphicsState->activeShader->uniformMaterial.diffuseVec4, material->diffuse[0], material->diffuse[1], material->diffuse[2], material->diffuse[3]);
		glUniform4f(graphicsState->activeShader->uniformMaterial.specularVec4, material->specular[0], material->specular[1], material->specular[2], material->specular[3]);
		glUniform1i(graphicsState->activeShader->uniformMaterial.shininessInt, material->shininess);
	}

	error = glGetError();
	// Save old matrix to the stack
	Matrix4d tmp = graphicsState->modelMatrixD;

	// Apply transformation
	graphicsState->modelMatrixD.Multiply(transformationMatrix);
	graphicsState->modelMatrixF = graphicsState->modelMatrixD;
	Matrix4f modelView = graphicsState->viewMatrixF * graphicsState->modelMatrixF;
	// Set uniform matrix in shader to point to the AppState modelView matrix.
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelView.getPointer());
	if (graphicsState->activeShader)
		glUniformMatrix4fv(graphicsState->activeShader->uniformModelMatrix, 1, false, graphicsState->modelMatrixF.getPointer());
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
			Vector3f position = Vector3f(mesh->vertices[face->vertices[j]].x, mesh->vertices[face->vertices[j]].y, mesh->vertices[face->vertices[j]].z);
			position = graphicsState->modelMatrixF * position;
			position = graphicsState->viewMatrixF * position;
			position = graphicsState->projectionMatrixF * position;

			glNormal3f(mesh->normals[face->normals[j]].x, mesh->normals[face->normals[j]].y, mesh->normals[face->normals[j]].z);
			glTexCoord2f(mesh->uvs[face->uvs[j]].x, mesh->uvs[face->uvs[j]].y);
			glVertex3f(mesh->vertices[face->vertices[j]].x, mesh->vertices[face->vertices[j]].y, mesh->vertices[face->vertices[j]].z);
		}
		glEnd();
	}

	++graphicsState->renderedObjects;		// increment rendered objects for debug info

	// Render children if needed
	if (child)
		for (int i = 0; i < children; ++i)
			child[i]->RenderOld(graphicsState);

	// Revert the model matrix to the old one in the stack
	graphicsState->modelMatrixD = tmp;
}

/// Rendering method
void Entity::Render(GraphicsState * graphicsState)
{
	if (graphics)
	{
		if (graphics->visible == false)
			return;
		// Skip if it is to be filtered with the current camera too.
		else if (graphics->cameraFilter.Exists(graphicsState->camera))
			return;
	}
	else if (graphicsState->settings & USE_LEGACY_GL){
		RenderOld(graphicsState);
		return;
	}
	
	int error = 0;

	// To send to the shadar
	int texturesToApply = 0;

	// Find diffuseMap!
	if (this->graphics && this->graphics->hasAnimation){

		diffuseMap = this->graphics->GetTextureForCurrentFrame(graphicsState->currentFrameTime);
	//	diffuseMap = TexMan.GetTextureByName("Checkpoints/test");
	//	diffuseMap = TexMan.GetTextureByName("RuneRPG/Units/200");
	//	diffuseMap = TexMan.GetTextureByName("logo8");
		if (diffuseMap->glid == -1){
			/// Bufferize it
			TexMan.BufferizeTexture(diffuseMap);
		}
	}

	// Bind texture if it isn't already bound.
	if (diffuseMap && graphicsState->currentTexture == diffuseMap){
		texturesToApply |= DIFFUSE_MAP;
	}
	else if (diffuseMap && graphicsState->currentTexture != diffuseMap){
		texturesToApply |= DIFFUSE_MAP;
		// When rendering an object with this program.
		glActiveTexture(GL_TEXTURE0 + 0);		// Select server-side active texture unit

		// Bind texture
		glBindTexture(GL_TEXTURE_2D, diffuseMap->glid);
		error = glGetError();
		
		// Bufferize it if queued? Should maybe move bufferization mechanism for textures to the TextureManager to be processed before a frame is rendered?
		if (diffuseMap->queueRebufferization)
		{
			diffuseMap->Bufferize();
		}

		// Set sampler in client graphicsState
		if (graphicsState->activeShader->uniformBaseTexture != -1)
			glUniform1i(graphicsState->activeShader->uniformBaseTexture, 0);		// Sets sampler
		error = glGetError();
	// Assign material
		// Bind sampler
		/// Core since 3.3!
		// glBindSampler(0, StateMan.sampler[0]);				// Sets sampler graphicsState
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		graphicsState->currentTexture = diffuseMap;
	}
	else if (diffuseMap == NULL){
		glActiveTexture(GL_TEXTURE0 + 0);		// Select server-side active texture unit
		glBindTexture(GL_TEXTURE_2D, NULL);
		graphicsState->currentTexture = NULL;
	}

	/// Bind specular if it isn't already o-o
	if (specularMap){
		texturesToApply |= SPECULAR_MAP;
		glActiveTexture(GL_TEXTURE0 + 1);		// Select server-side active texture unit

		// Bind texture
		glBindTexture(GL_TEXTURE_2D, specularMap->glid);
		error = glGetError();

		// Set sampler in client graphicsState
		if (graphicsState->activeShader->uniformSpecularMap!= -1)
			glUniform1i(graphicsState->activeShader->uniformSpecularMap, 1);		// Sets sampler
		error = glGetError();
	// Assign material
		// Bind sampler
		/// Core since 3.3!
		// glBindSampler(0, StateMan.sampler[0]);				// Sets sampler graphicsState
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		graphicsState->currentSpecularMap = specularMap;
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
		if (graphicsState->activeShader->uniformNormalMap != -1)
			glUniform1i(graphicsState->activeShader->uniformNormalMap, 2);		// Sets sampler
		error = glGetError();
	// Assign material
		// Bind sampler
		/// Core since 3.3!
		// glBindSampler(0, StateMan.sampler[0]);				// Sets sampler graphicsState
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		graphicsState->currentNormalMap = normalMap;
		glActiveTexture(GL_TEXTURE0 + 0);

	}
	else {
		glActiveTexture(GL_TEXTURE0 + 2);		// Select server-side active texture unit
		glBindTexture(GL_TEXTURE_2D, NULL);
		glActiveTexture(GL_TEXTURE0 + 0);
	//	glBindTexture(GL_TEXTURE_2D, NULL);
	}


	if (texturesToApply & DIFFUSE_MAP)
		glUniform1i(graphicsState->activeShader->uniformUseDiffuseMap, 1);
	else
		glUniform1i(graphicsState->activeShader->uniformUseDiffuseMap, 0);

	if (texturesToApply & SPECULAR_MAP)
		glUniform1i(graphicsState->activeShader->uniformUseSpecularMap, 1);
	else
		glUniform1i(graphicsState->activeShader->uniformUseSpecularMap, 0);

	if (texturesToApply & NORMAL_MAP)
		glUniform1i(graphicsState->activeShader->uniformUseNormalMap, 1);
	else
		glUniform1i(graphicsState->activeShader->uniformUseNormalMap, 0);

	// Send in texture-data to shader!
	// Set uniform matrix in shader to point to the AppState modelView matrix.
/*	GLuint uniform = glGetUniformLocation(graphicsState->activeShader->shaderProgram, "texturesToApply");
	if (uniform != -1)
		glUniform1i(uniform, texturesToApply);
*/

	error = glGetError();
	// Assign material
	// glProgramUniform Core since version 	4.1!
	// Use glUniform! Core since 2.0
//	glUniform4f(graphicsState->activeShader->uniformMaterial.ambientVec4, material->ambient[0], material->ambient[1], material->ambient[2], material->ambient[3]);
//	glUniform4f(graphicsState->activeShader->uniformMaterial.diffuseVec4, material->diffuse[0], material->diffuse[1], material->diffuse[2], material->diffuse[3]);
//	glUniform4f(graphicsState->activeShader->uniformMaterial.specularVec4, material->specular[0], material->specular[1], material->specular[2], material->specular[3]);
//	glUniform1i(graphicsState->activeShader->uniformMaterial.shininessInt, material->shininess);

	error = glGetError();
	// Save old matrix to the stack
	Matrix4d tmp = graphicsState->modelMatrixD;
	/// Recalculate matrix right before rendering.
//	RecalculateMatrix();

	/// If we have any offsets, apply them and re-calculate the transform matrix if so.
	if (graphics && graphics->renderOffset.MaxPart())
	{
		Vector3f offsetPosition = position + graphics->renderOffset;
		Matrix4d transMat = RecalculateMatrix(offsetPosition, rotation, scale); 
		graphicsState->modelMatrixD.Multiply(transMat);
		graphicsState->modelMatrixF = graphicsState->modelMatrixD;		
	}
	// Use standard matrix.
	else
	{
		// Apply transformation
		graphicsState->modelMatrixD.Multiply(transformationMatrix);
		graphicsState->modelMatrixF = graphicsState->modelMatrixD;		
	}
	// Set uniform matrix in shader to point to the AppState modelView matrix.
	glUniformMatrix4fv(graphicsState->activeShader->uniformModelMatrix, 1, false, graphicsState->modelMatrixF.getPointer());
	error = glGetError();

	// Set uniform matrix in shader to point to the AppState modelView matrix.
	GLuint uniform = glGetUniformLocation(graphicsState->activeShader->shaderProgram, "normalMatrix");
	Vector3f normals = Vector3f(0,1,0);
	Matrix4f normalMatrix = graphicsState->modelMatrixF.InvertedCopy().TransposedCopy();
	normals = normalMatrix.Product(normals);
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
	if (graphics && graphicsState->settings & ENABLE_SPECIFIC_ENTITY_OPTIONS){
		if (graphics->flags & RenderFlags::DISABLE_DEPTH_WRITE)
			glDepthMask(GL_FALSE);
		if (graphics->flags & RenderFlags::DISABLE_BACKFACE_CULLING)
			glDisable(GL_CULL_FACE);
		if (graphics->flags & RenderFlags::REQUIRES_DEPTH_SORTING){
			bool renderSortedEntities = graphicsState->settings & RENDER_SORTED_ENTITIES;
			if (!renderSortedEntities)
				requiresSorting = true;
		}

	}
	/// If requries sorting, save it in ze list
	if (requiresSorting){
		graphicsState->entitiesRequiringSorting.Add(this);
		render = false;
	}
	// Only render if previous states say so.
	if (render && model)
	{
		// Render the model
		model->GetTriangulatedMesh()->Render();
		++graphicsState->renderedObjects;		// increment rendered objects for debug info


		
		/// Debug-rendering to visualize differences between pre and post correction!
		if (physics && physics->estimator && physics->estimator->estimationMode == EstimationMode::EXTRAPOLATION_PLUS_COLLISION_CORRECTION)
		{
			Vector3f pos = position;
			position = physics->estimator->Position();
			Vector3f diff = pos - position;
			/// Only render other part if we get a difference.
			if (true /*diff.MaxPart()*/){
				RecalculateMatrix();
				// Reload matrix into shader. First grab old matrix.
				graphicsState->modelMatrixD = tmp;
				// Apply transformation
				graphicsState->modelMatrixD.Multiply(transformationMatrix);
				graphicsState->modelMatrixF = graphicsState->modelMatrixD;
				// Set uniform matrix in shader to point to the AppState modelView matrix.
				glUniformMatrix4fv(graphicsState->activeShader->uniformModelMatrix, 1, false, graphicsState->modelMatrixF.getPointer());
				error = glGetError();
				/// Set other render-mode so we know what we're looking at.
				glBlendFunc(GL_ONE, GL_ONE);
				glDisable(GL_DEPTH_TEST);
				// Re-paint
				model->GetTriangulatedMesh()->Render();
				++graphicsState->renderedObjects;		// increment rendered objects for debug info
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_DEPTH_TEST);
			}
			/// Reset matrix to the old one to not fuck up camera movement later on?
			position = pos;
			RecalculateMatrix();
		}
	}
	// Disable any modifiers now, unless we got a modifier that tells us to apply the previous modifiers to the children as well.
	if (graphics && graphicsState->settings & ENABLE_SPECIFIC_ENTITY_OPTIONS){
		if (graphics->flags & RenderFlags::DISABLE_DEPTH_WRITE)
			glDepthMask(GL_TRUE);
		if (graphics->flags & RenderFlags::DISABLE_BACKFACE_CULLING)
			glEnable(GL_CULL_FACE);
	}

	// Render children if needed
	if (child)
		for (int i = 0; i < children; ++i)
			child[i]->Render(graphicsState);

	// Revert the model matrix to the old one in the stack
	graphicsState->modelMatrixD = tmp;

	/// Stock up graphical effects to render if any
	if (graphics != NULL){
		if (graphics->effects != NULL)
			graphicsState->graphicEffectsToBeRendered += *graphics->effects;
        if (graphics->particleSystems != NULL)
            graphicsState->particleEffectsToBeRendered += *graphics->particleSystems;
		// Register dynamic lights if needed
		if (graphics->dynamicLights)
		{
			for (int i = 0; i < graphics->dynamicLights->Size(); ++i)
			{
				Light * light = (*graphics->dynamicLights)[i];
				if (!light->registeredForRendering)
				{
					graphicsState->dynamicLights.Add(light);
					light->registeredForRendering = true;
				}
			}
		}
		
		/// If we have any text, render it last!
		if (graphics->text)
		{
			Matrix4f & modelMatrix = graphicsState->modelMatrixF;
			modelMatrix = Matrix4f();


			modelMatrix.Multiply((Matrix4d().Translate(Vector3d(position))));
			modelMatrix.Multiply(rotationMatrix);
			modelMatrix.Multiply((Matrix4d().Scale(Vector3d(scale * graphics->textSizeRatio))));

			TextFont * font = graphicsState->currentFont;
			Text & textToRender = graphics->text;

			Vector4f textColor = graphics->textColor;
			glColor4f(textColor.x, textColor.y, textColor.z, textColor.w);
			font->SetColor(textColor);
			
			// Calcualte size it would assume with current matrix.
			Vector2f textRenderSize = font->CalculateRenderSizeWorldSpace(textToRender, graphicsState);
			
			// Recalculate the matrix to center the text on the entity.
			modelMatrix = Matrix4d();
			Vector3f centeringOffset(- textRenderSize.x * 0.5f, textRenderSize.y, 0);
			modelMatrix.Multiply((Matrix4d().Translate(Vector3d(position + centeringOffset))));
			modelMatrix.Multiply(rotationMatrix);
			modelMatrix.Multiply((Matrix4d().Scale(Vector3d(scale * graphics->textSizeRatio))));

			

			font->RenderText(textToRender, graphicsState);

			
#ifdef DEBUG_TRIANGLE
			glUseProgram(0);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glLoadMatrixf(graphicsState->projectionMatrixF.getPointer());
			Matrix4f modelView = graphicsState->viewMatrixF * graphicsState->modelMatrixF;
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(modelView.getPointer());
			glEnable(GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
			glColor4f(0,1,1,1);
	
			glDisable(GL_TEXTURE_2D);
		//	glDisable(GL_COLOR_MATERIAL);
			glDisable(GL_LIGHTING);

			glDisable(GL_DEPTH_TEST);
			// Draw some more random shit?
			glBegin(GL_TRIANGLES);
				glVertex3f(0,0,0);
				glVertex3f(5,0,0);
				glVertex3f(0,5,0);
			glEnd();
			glUseProgram(graphicsState->activeShader->shaderProgram);
#endif
		
		}
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
void Entity::SetPosition(Vector3f position)
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
void Entity::RotateGlobal(Quaternion withQuaternion)
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
void Entity::Rotate(Vector3f rotation)
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
void Entity::SetRotation(Quaternion quat)
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
	

void Entity::SetRotation(Vector3f rotation)
{
//	assert(false);
	this->rotation = rotation;
	if (physics && physics->useQuaternions)
	{
		physics->orientation = Quaternion();
		Quaternion rotationQuaternion = Quaternion(rotation, 1.0f);
		rotationQuaternion.Normalize();
		physics->orientation = physics->orientation * rotationQuaternion;
		physics->orientation.Normalize();
	}
	RecalculateMatrix();
}

/// Sets scale of the entity
void Entity::SetScale(Vector3f scale)
{
	assert(scale.x && scale.y && scale.z);
	this->scale = scale;
	RecalculateMatrix();
}
/// Scales the Entity
void Entity::Scale(Vector3f scale){
	this->scale = Vector3f(this->scale.x * scale.x, this->scale.y * scale.y, this->scale.z * scale.z);
	RecalculateMatrix();
}
/// Scales the Entity
void Entity::Scale(float scale){
	this->scale = Vector3f(this->scale.x * scale, this->scale.y * scale, this->scale.z * scale);
	RecalculateMatrix();
}
/// Translates the Entity
void Entity::Translate(float x, float y, float z){
	this->position.x += x;
	this->position.y += y;
	this->position.z += z;
	RecalculateMatrix();
}
/// Translates the Entity
void Entity::Translate(Vector3f translation){
	this->position =  this->position + translation;
	RecalculateMatrix();
}

/// Recalculates the transformation matrix
void Entity::RecalculateMatrix()
{

    rotationMatrix = Matrix4d();
		
	// Quaternions for those entities wanting to use it.
    if (physics && physics->useQuaternions){
        Quaternion q = physics->orientation;
     //   std::cout<<"\nQ preN: "<<q;
        q.Normalize();
     //   std::cout<<"\nQ postN: "<<q;
        rotationMatrix = q.Matrix();
        double * parr = rotationMatrix.getPointer();
     //   std::cout<<"\nMatrix: "<<parr[0]<<" "<<parr[1];
    }
	// Euclidean co-ordinates.
	else 
	{
		rotationMatrix.Multiply(Matrix4d::GetRotationMatrixX(rotation.x));
		rotationMatrix.Multiply(Matrix4d::GetRotationMatrixY(rotation.y));
		rotationMatrix.Multiply(Matrix4d::GetRotationMatrixZ(rotation.z));
	}

	transformationMatrix = Matrix4d();

	transformationMatrix.Multiply((Matrix4d().Translate(Vector3d(position))));
	transformationMatrix.Multiply(rotationMatrix);
	transformationMatrix.Multiply((Matrix4d().Scale(Vector3d(scale))));

		// Ensure it has a scale..?
//	assert(transformationMatrix.HasValidScale());
}

/// Recalculates a transformation matrix using argument vectors for position, rotation and translation.
Matrix4f Entity::RecalculateMatrix(Vector3f & position, Vector3f & rotation, Vector3f & scale)
{
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
		if (position.LengthSquared() > 1.0f || scale.LengthSquared() > 1.0f)
			;//assert(prevPos.x != newPos.x);
	}
	return triangles;
}
