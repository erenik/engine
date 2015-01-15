/// Emil Hedemalm
/// 2014-09-23
/// Debug-renderer for skeletons and animation data.

#include "../GraphicsManager.h"

#include "ShaderManager.h"

#include "Graphics/GraphicsProperty.h"

#include "Entity/Entity.h"
#include "Model/Model.h"
#include "Mesh/Mesh.h"
#include "Model/SkeletalAnimationNode.h"

/// Re-renders the current scene, but this time illustrating Skeleton/Animation data.
void GraphicsManager::RenderSkeletons()
{
	ShadeMan.SetActiveShader(0);
//	rer
	// Load projection matrix.

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(graphicsState->projectionMatrixF.getPointer());
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(graphicsState->viewMatrixF.getPointer());

	// Load view matrix.


	// Set gl states so that we actually can debug-render anything.
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_COLOR_MATERIAL);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glLineStipple(1, 0x0101);
	glLineWidth(1.0f);
	
	glColor4f(1,1,1,1);


	int64 milliseconds = Time::Now().Milliseconds();

	for (int i = 0; i < registeredEntities.Size(); ++i)
	{
		Entity * entity = registeredEntities[i];
		Model * model = entity->model;
		if (!model)
			continue;
		Mesh * mesh = model->mesh;
		graphicsState->modelMatrixF = entity->transformationMatrix;
		Bone * skeleton = mesh->skeleton;
		if (skeleton && true)
		{
			/// Animate it if true?
			if (entity->graphics->skeletalAnimationEnabled)
			{
				/// Animate it, looped. 
		//		skeleton->AnimateForTime(milliseconds, true);
				// Short test with an identity matrix instead, or just the rotatoin matrix from the entity.
				Matrix4f identity;
			//	skeleton->UpdateMatrices(identity);
			//	skeleton->UpdateMatrices(entity->transformationMatrix);
			}

			skeleton->RenderBones(*graphicsState);
		}
	}

}
	

