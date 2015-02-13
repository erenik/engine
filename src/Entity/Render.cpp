/// Emil Hedemalm
/// 2014-09-12
/// Splitting up code, only render here.

#include "Entity.h"
#include "GraphicsState.h"
#include "TextureManager.h"
#include "Shader.h"
#include "Light.h"
#include "Model/Model.h"
#include "Mesh/Mesh.h"

#include "Graphics/Fonts/TextFont.h"
#include "Graphics/GraphicsProperty.h"

#include "Physics/PhysicsProperty.h"

#include "ShaderManager.h"

/// Rendering method
void Entity::Render(GraphicsState & graphicsState)
{
	// If rendering shadows, skip relevant entities.
	if (graphics)
	{
		if (graphics->castsShadow && graphicsState.shadowPass)
			return;
		if (graphics->visible == false)
			return;
		// Skip if it is to be filtered with the current camera too.
		else if (graphics->cameraFilter.Exists(graphicsState.camera))
			return;
	}
	Shader * shader = ShaderMan.ActiveShader();
	if (!shader)
	{
		return;
	}

	// Change shader to one supporting skeletal animations if necessary.
	if (this->graphics->skeletalAnimationEnabled && this->graphics->shaderBasedSkeletonAnimation && shader->attributeBoneIndices == -1)
	{
		shader = ShadeMan.SetActiveShader("PhongSkeletal");
		if (!shader)
			return;
		// Load in the main matrices.
		glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, graphicsState.projectionMatrixF.getPointer());
		glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, graphicsState.viewMatrixF.getPointer());
	}

	if (graphicsState.settings & USE_LEGACY_GL){
		RenderOld(graphicsState);
		return;
	}
	
	int error = 0;

	// To send to the shadar
	int texturesToApply = 0;

	// Find diffuseMap!
	if (this->graphics && this->graphics->hasAnimation)
	{
		diffuseMap = this->graphics->GetTextureForCurrentFrame(graphicsState.frametimeStartMs);
		assert(diffuseMap);
	//	diffuseMap = TexMan.GetTextureByName("Checkpoints/test");
	//	diffuseMap = TexMan.GetTextureByName("RuneRPG/Units/200");
	//	diffuseMap = TexMan.GetTextureByName("logo8");
		if (diffuseMap->glid == -1)
		{
			/// Bufferize it
			TexMan.BufferizeTexture(diffuseMap);
		}
	}

	// Bind texture if it isn't already bound.
	if (diffuseMap && graphicsState.currentTexture == diffuseMap){
		texturesToApply |= DIFFUSE_MAP;
	}
	else if (diffuseMap && graphicsState.currentTexture != diffuseMap)
	{
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
		if (shader->uniformBaseTexture != -1)
			glUniform1i(shader->uniformBaseTexture, 0);		// Sets sampler
		error = glGetError();
	// Assign material
		// Bind sampler
		/// Core since 3.3!
		// glBindSampler(0, StateMan.sampler[0]);				// Sets sampler graphicsState
		
		/// Sets glTExParameter for Min/Mag filtering
		diffuseMap->SetSamplingMode();
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
		if (shader->uniformSpecularMap!= -1)
			glUniform1i(shader->uniformSpecularMap, 1);		// Sets sampler
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
		if (shader->uniformNormalMap != -1)
			glUniform1i(shader->uniformNormalMap, 2);		// Sets sampler
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
		glUniform1i(shader->uniformUseDiffuseMap, 1);
	else
		glUniform1i(shader->uniformUseDiffuseMap, 0);

	if (texturesToApply & SPECULAR_MAP)
		glUniform1i(shader->uniformUseSpecularMap, 1);
	else
		glUniform1i(shader->uniformUseSpecularMap, 0);

	if (texturesToApply & NORMAL_MAP)
		glUniform1i(shader->uniformUseNormalMap, 1);
	else
		glUniform1i(shader->uniformUseNormalMap, 0);

	// Send in texture-data to shader!
	// Set uniform matrix in shader to point to the AppState modelView matrix.
/*	GLuint uniform = glGetUniformLocation(shader->shaderProgram, "texturesToApply");
	if (uniform != -1)
		glUniform1i(uniform, texturesToApply);
*/

	error = glGetError();
	// Assign material
	// glProgramUniform Core since version 	4.1!
	// Use glUniform! Core since 2.0
//	glUniform4f(shader->uniformMaterial.ambientVec4, material->ambient[0], material->ambient[1], material->ambient[2], material->ambient[3]);
//	glUniform4f(shader->uniformMaterial.diffuseVec4, material->diffuse[0], material->diffuse[1], material->diffuse[2], material->diffuse[3]);
//	glUniform4f(shader->uniformMaterial.specularVec4, material->specular[0], material->specular[1], material->specular[2], material->specular[3]);
//	glUniform1i(shader->uniformMaterial.shininessInt, material->shininess);

	error = glGetError();
	// Save old matrix to the stack
	Matrix4d tmp = graphicsState.modelMatrixD;
	/// Recalculate matrix right before rendering.
//	RecalculateMatrix();

	/// If we have any offsets, apply them and re-calculate the transform matrix if so.
	if (graphics && graphics->renderOffset.MaxPart())
	{
		Vector3f offsetPosition = position + graphics->renderOffset;
		Matrix4d transMat = RecalculateMatrix(offsetPosition, rotation, scale); 
		graphicsState.modelMatrixD.Multiply(transMat);
		graphicsState.modelMatrixF = graphicsState.modelMatrixD;		
	}
	// Use standard matrix.
	else
	{
		// Apply transformation
		graphicsState.modelMatrixD.Multiply(transformationMatrix);
		graphicsState.modelMatrixF = graphicsState.modelMatrixD;		
	}
	// Set uniform matrix in shader to point to the AppState modelView matrix.
	glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, graphicsState.modelMatrixF.getPointer());
	error = glGetError();

	// Set uniform matrix in shader to point to the AppState modelView matrix.
	GLuint uniform = glGetUniformLocation(shader->shaderProgram, "normalMatrix");
	Vector3f normals = Vector3f(0,1,0);
	Matrix4f normalMatrix = graphicsState.modelMatrixF.InvertedCopy().TransposedCopy();
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
	if (graphics && graphicsState.settings & ENABLE_SPECIFIC_ENTITY_OPTIONS)
	{
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
	/// TODO: Add a query if an entity should be sorted and let the render-passes the render pipeline handle them as needed.
	/// If requries sorting, save it in ze list
	/*
	if (requiresSorting)
	{
		graphicsState.entitiesRequiringSorting.Add(this);
		render = false;
	}
	*/
	// Only render if previous states say so.
	if (render && model)
	{
		// Set blend-mode
		glBlendFunc(graphics->blendModeSource, graphics->blendModeDest);
		// Set depth test
		if (graphics->depthTest)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		// Set multiplicative base color (1,1,1,1) default.
		glUniform4f(shader->uniformPrimaryColorVec4, graphics->color[0], graphics->color[1], graphics->color[2], graphics->color[3]);

		// Bind bone-specific buffers as needed.
		if (graphics->skeletalAnimationEnabled && graphics->shaderBasedSkeletonAnimation)
		{
			Mesh * mesh = model->GetTriangulatedMesh();
			// Bind bone indices!
			glBindBuffer(GL_ARRAY_BUFFER, mesh->boneIndexBuffer);
			static const GLint offsetBoneIndices = 4 * sizeof(GLint);
			glVertexAttribIPointer(shader->attributeBoneIndices, 4, GL_INT, offsetBoneIndices, 0);		// Integers!
			// Bind bone weights!
			glBindBuffer(GL_ARRAY_BUFFER, mesh->boneWeightsBuffer);
			static const GLint offsetBoneWeights = 4 * sizeof(GLfloat);
			glVertexAttribPointer(shader->attributeBoneWeights, 4, GL_FLOAT, GL_FALSE, offsetBoneWeights, 0);		// Floats!
			// Find index of the bone skinning matrix map sampler!

			// Update the bone skinning map as needed.. e.e
		//	model->UpdateSkinningMatrixMap();
			// Bufferize original positions!
			mesh->Bufferize(true, false);

			// Set sampler index to use for the skinning matrix map.
			glActiveTexture(GL_TEXTURE0 + 3);

			// Bind the texture filled with bone transformation matrix data.
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glActiveTexture(GL_TEXTURE0 + 3);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, model->boneSkinningMatrixMap);

			// Set parameters!
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			CheckGLError("lalaalall");
			
			// Revert to default texture index after binding is complete.
			glActiveTexture(GL_TEXTURE0 + 0);
		//	glBindTexture(GL_TEXTURE_1D, mesh->boneMatrixMap);
		}

		// Render the model
		model->Render(graphicsState);
		++graphicsState.renderedObjects;		// increment rendered objects for debug info


		//
		///// Debug-rendering to visualize differences between pre and post correction!
		//if (physics && physics->estimator && physics->estimator->estimationMode == EstimationMode::EXTRAPOLATION_PLUS_COLLISION_CORRECTION)
		//{
		//	Vector3f pos = position;
		//	position = physics->estimator->Position();
		//	Vector3f diff = pos - position;
		//	/// Only render other part if we get a difference.
		//	if (true /*diff.MaxPart()*/){
		//		RecalculateMatrix();
		//		// Reload matrix into shader. First grab old matrix.
		//		graphicsState.modelMatrixD = tmp;
		//		// Apply transformation
		//		graphicsState.modelMatrixD.Multiply(transformationMatrix);
		//		graphicsState.modelMatrixF = graphicsState.modelMatrixD;
		//		// Set uniform matrix in shader to point to the AppState modelView matrix.
		//		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, graphicsState.modelMatrixF.getPointer());
		//		error = glGetError();
		//		/// Set other render-mode so we know what we're looking at.
		//		glBlendFunc(GL_ONE, GL_ONE);
		//		glDisable(GL_DEPTH_TEST);
		//		// Re-paint
		//		model->GetTriangulatedMesh()->Render();
		//		++graphicsState.renderedObjects;		// increment rendered objects for debug info
		//		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//		glEnable(GL_DEPTH_TEST);
		//	}
		//	/// Reset matrix to the old one to not fuck up camera movement later on?
		//	position = pos;
		//	RecalculateMatrix();
		//}
	}
	// Disable any modifiers now, unless we got a modifier that tells us to apply the previous modifiers to the children as well.
	if (graphics && graphicsState.settings & ENABLE_SPECIFIC_ENTITY_OPTIONS){
		if (graphics->flags & RenderFlags::DISABLE_DEPTH_WRITE)
			glDepthMask(GL_TRUE);
		if (graphics->flags & RenderFlags::DISABLE_BACKFACE_CULLING)
			glEnable(GL_CULL_FACE);
	}

	// Render children if needed
	// No, children are rendered independently!
	/*
	if (child)
		for (int i = 0; i < children; ++i)
			child[i]->Render(graphicsState);
*/

	// Revert the model matrix to the old one in the stack
	graphicsState.modelMatrixD = tmp;

	/// Stock up graphical effects to render if any
	if (graphics != NULL)
	{
		graphicsState.graphicEffectsToBeRendered += graphics->effects;
        graphicsState.particleEffectsToBeRendered += graphics->particleSystems;
		// Register dynamic lights if needed
		for (int i = 0; i < graphics->dynamicLights.Size(); ++i)
		{
			Light * light = graphics->dynamicLights[i];
			assert(light);
			if (!light->registeredForRendering)
			{
				graphicsState.dynamicLights.Add(light);
				light->registeredForRendering = true;
			}
		}
		
		/// If we have any text, render it last!
		TextFont * font = graphicsState.currentFont;
		if (graphics->text.Length() && font)
		{
			Matrix4f & modelMatrix = graphicsState.modelMatrixF;
			modelMatrix = Matrix4f();


			modelMatrix.Multiply((Matrix4d().Translate(Vector3d(position))));
			modelMatrix.Multiply(rotationMatrix);
			modelMatrix.Multiply((Matrix4d().Scale(Vector3d(scale * graphics->textSizeRatio))));

			Text & textToRender = graphics->text;

			Vector4f textColor = graphics->textColor;
			glColor4f(textColor[0], textColor[1], textColor[2], textColor[3]);
			font->SetColor(textColor);
			
			// Calcualte size it would assume with current matrix.
			Vector2f textRenderSize = font->CalculateRenderSizeWorldSpace(textToRender, graphicsState);
			
			// Recalculate the matrix to center the text on the entity.
			modelMatrix = Matrix4d();
			Vector3f centeringOffset(- textRenderSize[0] * 0.5f, textRenderSize[1], 0);
			modelMatrix.Multiply((Matrix4d().Translate(Vector3d(position + centeringOffset))));
			modelMatrix.Multiply(rotationMatrix);
			modelMatrix.Multiply((Matrix4d().Scale(Vector3d(scale * graphics->textSizeRatio))));

			

			font->RenderText(textToRender, graphicsState);

			
#ifdef DEBUG_TRIANGLE
			ShadeMan.SetActiveShader(0);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glLoadMatrixf(graphicsState.projectionMatrixF.getPointer());
			Matrix4f modelView = graphicsState.viewMatrixF * graphicsState.modelMatrixF;
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
			glUseProgram(shader->shaderProgram);
#endif
		
		}
	}
}
