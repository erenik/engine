// Emil Hedemalm
// 2013-07-20

#include "GraphicsState.h"
#include "Lighting.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/GraphicsProperty.h"
#include "Entity/Entity.h"
#include "Render/RenderInstancingGroup.h"
#include "File/LogFile.h"

/** Main state for rendering. Contains settings for pretty much everything which is not embedded in other objects.
	Read only unless you know what you're doing (and are located within a render-thread function).
*/
GraphicsState::GraphicsState()
{
	LogMain("GraphicsState created", INFO);

	defaultEntityGroup = new EntityGroup();
	perFrameSmoothness = 0.7f;
	shadowPass = false;
	activeWindow = NULL;
	activeViewport = NULL;
//	activeShader = NULL;
	currentMesh = NULL;
	currentTexture = NULL;
	currentSpecularMap = NULL;
	currentNormalMap = NULL;
	currentFont = NULL;
	gridSpacing = 10.0f;
	gridSize = 20;
	camera = NULL;
	settings = 0;
	optimizationLevel = 0;
	viewportX0 = viewportY0 = 0;
	promptScreenshot = recording = false;
	screenshotsTaken = 0;
	framesRecorded = 0;

	flags = 0;
	fogBegin = 500.0f;
	fogEnd = 2500.0f;

	boundVertexArrayBuffer = 0;

	resolutionLocked = false;
	renderResolution = Vector2i(32,32);
	relativeResolution = Vector2f(1,1);
	antialiasing = true;
	farPlane = 0;
};

GraphicsState::~GraphicsState()
{
	/// Delete RenderInstancingGroups
	shadowCastingEntityGroups.ClearAndDelete();
	entityGroups.ClearAndDelete();
}


/// Adds an entity to the graphics state, which includes sorting it into proper instancing groups, if flagged for it.
void GraphicsState::AddEntity(Entity* entity)
{
	GraphicsProperty * gp = entity->graphics;
	if (gp->group.Length())
	{
		bool added = false;
		for (int i = 0; i < entityGroups.Size(); ++i)
		{
			EntityGroup * eg = entityGroups[i];
			if (eg->name == gp->group)
			{
				eg->AddItem(entity);
				added = true;
			}
		}
		if (!added)
		{
			EntityGroup * eg = new EntityGroup();
			eg->name = gp->group;
			eg->AddItem(entity);
			entityGroups.AddItem(eg);
		}
	}
	else
		defaultEntityGroup->AddItem(entity);
	/// Add alpha-entities straight to the graphics-state?
	if (gp->flags & RenderFlag::ALPHA_ENTITY)
	{
		alphaEntities.AddItem(entity);
	}
	else 
	{
		solidEntities.AddItem(entity);
		if (!gp->renderInstanced)
			solidEntitiesNotInstanced.AddItem(entity);
	}
	/// Shadow groups
	if (gp->castsShadow)
	{
		shadowCastingEntities.AddItem(entity);
		// Add to instancing groups as requested based on the given boolean flags.
		if (!gp->renderInstanced)
		{
			shadowCastingEntitiesNotInstanced.AddItem(entity);
		}
	}
	/// Setup instance render groups if needed.
	if (!gp->renderInstanced)
		return;
	RIG * rig = GetGroup(entityInstancingGroups, entity);
	if (rig)
		rig->AddEntity(entity);
	else
	{
		rig = new RIG(entity);
		entityInstancingGroups.AddItem(rig);
		if (rig->isSolid)
			solidEntityGroups.AddItem(rig);
		if (rig->isShadowCasting)
			shadowCastingEntityGroups.AddItem(rig);
	}	
}

void GraphicsState::RemoveEntity(Entity* entity)
{
	GraphicsProperty * gp = entity->graphics;
	if (!gp)
		return;
	if (gp->group.Length())
	{
		for (int i = 0; i < entityGroups.Size(); ++i)
		{
			EntityGroup * eg = entityGroups[i];
			if (eg->name == gp->group)
			{
				eg->RemoveItem(entity);
			}
		}
	}
	else
		defaultEntityGroup->RemoveItem(entity);
	/// Specific groups.
	if (gp->flags & RenderFlag::ALPHA_ENTITY)
		alphaEntities.RemoveItemUnsorted(entity);
	else  
	{
		solidEntities.RemoveItemUnsorted(entity);
		if (!gp->renderInstanced)
			solidEntitiesNotInstanced.RemoveItemUnsorted(entity);
	}
	if (gp->castsShadow)
	{
		shadowCastingEntities.RemoveItemUnsorted(entity);
		if (!gp->renderInstanced)
			shadowCastingEntitiesNotInstanced.RemoveItemUnsorted(entity);
	}

	/// Remove from instancing groups as needed.
	if (gp->renderInstanced)
	{
		for (int i = 0; i < entityInstancingGroups.Size(); ++i)
		{
			RIG * rig = entityInstancingGroups[i];
			rig->RemoveEntity(entity);
			if (rig->reference == 0)
			{
				entityInstancingGroups.RemoveItemUnsorted(rig);
				shadowCastingEntityGroups.RemoveItemUnsorted(rig);
				solidEntityGroups.RemoveItemUnsorted(rig);
				delete rig;
				--i;
			}
		}
	}
}

void GraphicsState::RemoveAllEntities()
{
	defaultEntityGroup->Clear();
	entityGroups.ClearAndDelete();
	alphaEntities.Clear();
	solidEntities.Clear();
	solidEntitiesNotInstanced.Clear();
	shadowCastingEntities.Clear();
	shadowCastingEntitiesNotInstanced.Clear();
	
	entityInstancingGroups.ClearAndDelete();
	shadowCastingEntityGroups.Clear(); // Same pointers here
	solidEntityGroups.Clear(); // and here, thus clear.
}

void GraphicsState::UpdateRenderInstancingGroupBuffers()
{
	for (int i = 0; i < entityInstancingGroups.Size(); ++i)
	{
		RIG * rig = entityInstancingGroups[i];
		rig->UpdateBuffers();
	}
}


RenderInstancingGroup * GraphicsState::GetGroup(List<RenderInstancingGroup*> & fromListOfGroups, Entity* entity)
{
	for (int i = 0; i < fromListOfGroups.Size(); ++i)
	{
		RIG * rig = fromListOfGroups[i];
		// Compare with reference entity.
		Entity* reference = rig->reference;
		if (!reference)
		{
			// Delete it?
			continue;
		}
		if (reference->model == entity->model)
		{
			return rig;
		}
	}
	return NULL;
}



/// Calls glScissor, and updates locally tracked scissor. Appends current viewport x0/y0 co-ordinates automatically to the GL call.
void GraphicsState::SetGLScissor(const Rect & newScissor)
{
	CheckGLError("GraphicsState::SetGLScissor 1");
	bool scissorDisabled = (settings & SCISSOR_DISABLED) > 0;
    if (scissorDisabled)
	{
		glDisable(GL_SCISSOR_TEST);
		return;
	}

	this->scissor = newScissor;
	Vector2i size = scissor.Size();
	glScissor((GLint)(scissor.mini[0] + viewportX0), (GLint)(scissor.mini[1] + viewportY0), size[0] < 0 ? 0 : size[0], size[1] < 0 ? 0 : size[1]);

	CheckGLError("GraphicsState::SetGLScissor 2");
}


void GraphicsState::SetCamera(Camera * camera)
{	
	this->camera = camera;
	projectionMatrixF = projectionMatrixD = camera->ProjectionMatrix4d();
	viewMatrixF = viewMatrixD = camera->ViewMatrix4d();
}			

/// Used to reduce amount of calls to GL by not re-binding and setting up all vertex buffer stats if the same buffer is still bound. GL_ARRAY_BUFFER type
int GraphicsState::BoundVertexArrayBuffer() const 
{
	return boundVertexArrayBuffer;
}

/// Used to reduce amount of calls to GL by not re-binding and setting up all vertex buffer stats if the same buffer is still bound. GL_ARRAY_BUFFER type
void GraphicsState::BindVertexArrayBuffer(int vertexBufferID)
{
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	this->boundVertexArrayBuffer = vertexBufferID;
}
