/// Emil Hedemalm
/// 2015-02-20
/// Structure for containing a group of entities to be rendered instanced 
/// (one draw command), after the necessary buffers have been updated.

#include "RenderInstancingGroup.h"
#include "GraphicsState.h"
#include "Globals.h"
#include "Graphics/GLBuffers.h"
#include "Model/Model.h"
#include "Mesh/Mesh.h"
#include "File/LogFile.h"
#include "ShaderManager.h"
#include "Texture.h"

RenderInstancingGroup::RenderInstancingGroup()
{
	Nullify();
}
RenderInstancingGroup::RenderInstancingGroup(Entity * fromReference)
{
	Nullify();
	AddEntity(fromReference);
}
void RenderInstancingGroup::Nullify()
{
	options = 0;
	matrixBuffer = -1;
	matrixData = NULL;
	bufferEntityLength = 0;
	reference = NULL;
}

RenderInstancingGroup::~RenderInstancingGroup()
{
	SAFE_DELETE_ARR(matrixData);
}

/// o.o
void RIG::AddEntity(Entity * entity)
{
	// Claiming a disowned empty group.
	if (reference == NULL)
		reference = entity;
	AddItem(entity);
}
/// Removes all occurences of any items in the sublist in this list. Also re-points the reference pointer if needed.
void RIG::RemoveEntity(Entity * entity)
{
	RemoveItemUnsorted(entity);
	if (entity == reference)
	{
		if (currentItems > 0)
			reference = arr[0];
		else
			reference = NULL;
	}
}


// Called once per frame.
void RenderInstancingGroup::UpdateBuffers(bool force)
{
	// Re-allocate matrixData buffer as needed.
	if (bufferEntityLength < this->arrLength)
	{
		// Update length. Add some more in case we need it later.
		bufferEntityLength = arrLength * 2;
		// Put a minimum.
		if (bufferEntityLength < 10)
			bufferEntityLength = 10;
		SAFE_DELETE(matrixData);
		matrixData = new float[bufferEntityLength * 16];
	}
	else 
	{
		if (!force)
			return;
	}
	// Fill data.
	for (int i = 0; i < this->currentItems; ++i)
	{
		Entity * entity = arr[i];
		float * matrPtr = entity->transformationMatrix.getPointer();
		/*
		for (int j = 0; j < 16; ++j)
		{
			matrixData[i * 16 + j] = ;
		}*/
		memcpy(&matrixData[i*16], matrPtr, 16 * sizeof(float));
	}
	for (int i = 0; i < currentItems * 16 && debug == -7; ++i)
	{
		std::cout<<"\ni "<<i<<": "<<matrixData[i];
	}
	/// Buffer in data into GL now, and not every render-frame?
	// The VBO containing the positions and sizes of the particles
	if (matrixBuffer == -1)
	{
		matrixBuffer = GLBuffers::New();
		glBindBuffer(GL_ARRAY_BUFFER, matrixBuffer); // Buffer once initially?
		glBufferData(GL_ARRAY_BUFFER, currentItems * 16 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	}
	// Buffer the actual data.
	glBindBuffer(GL_ARRAY_BUFFER, matrixBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, currentItems * 16 * sizeof(GLfloat), matrixData);
}

// Called once per viewport that is rendered.
void RenderInstancingGroup::Render()
{
	if (!reference) // empty group?
		return;
	Shader * shader = ActiveShader();
	if (debug == -3)
		return;
	// Don't mess with any states unless needed.
	// http://www.opengl.org/wiki/Buffer_Object_Streaming

	// Set up model properties first?
	reference->model->mesh->BindVertexBuffer();
	// 1rst attribute buffer : vertices
	graphicsState->BindVertexArrayBuffer(matrixBuffer);
	if (shader->attributeInstanceModelMatrix == -1)
	{
		LogGraphics("No instance ModelMatrix attribute found in current shader?", ERROR);
		return;
	}
	// Enable instancing in the shader.
	glUniform1i(shader->uniformInstancingEnabled, 1);
//	shader->PrintAttributes();
	// Enable the vertex array containing our matrix data.
	glEnableVertexAttribArray(shader->attributeInstanceModelMatrix);
	glEnableVertexAttribArray(shader->attributeInstanceModelMatrix+1);
	glEnableVertexAttribArray(shader->attributeInstanceModelMatrix+2);
	glEnableVertexAttribArray(shader->attributeInstanceModelMatrix+3);

	// These functions are specific to glDrawArrays*Instanced*.
	// The first parameter is the attribute buffer we're talking about.
	// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
	// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor[0]ml
	/// Mesh-data, re-use it all.
	glVertexAttribDivisor(shader->attributePosition, 0); 
	if (shader->attributeNormal != -1)
		glVertexAttribDivisor(shader->attributeNormal, 0); 
	if (shader->attributeUV != -1)
		glVertexAttribDivisor(shader->attributeUV, 0); 
	if (shader->attributeTangent != -1)
		glVertexAttribDivisor(shader->attributeTangent, 0); 
	
	CheckGLError("RenderInstancingGroup::Render - stuff");
	/// Per-entity data.
	static const GLint offsetBoneWeights = 4 * sizeof(GLfloat);
//	glVertexAttribPointer(shader->attributePosition, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * floatsPerVertex, 0);		// Position
	glVertexAttribPointer(shader->attributeInstanceModelMatrix, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, 0);	// Actually supply the array..
	CheckGLError("RenderInstancingGroup::Render - glVertexAttribPointer 1");
	static const GLint offset1 = 4 * sizeof(GLfloat);
	glVertexAttribPointer(shader->attributeInstanceModelMatrix + 1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, (void*)offset1);	// Actually supply the array..
	CheckGLError("RenderInstancingGroup::Render - glVertexAttribPointer 2");
	static const GLint offset2 = 2 * 4 * sizeof(GLfloat);
	glVertexAttribPointer(shader->attributeInstanceModelMatrix + 2, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, (void*)offset2);	// Actually supply the array..
	CheckGLError("RenderInstancingGroup::Render - glVertexAttribPointer 3");
	static const GLint offset3 = 3 * 4 * sizeof(GLfloat);
	glVertexAttribPointer(shader->attributeInstanceModelMatrix + 3, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, (void*)offset3);	// Actually supply the array..
	CheckGLError("RenderInstancingGroup::Render - glVertexAttribPointer 4");
	glVertexAttribDivisor(shader->attributeInstanceModelMatrix, 1); 
	glVertexAttribDivisor(shader->attributeInstanceModelMatrix+1, 1); 
	glVertexAttribDivisor(shader->attributeInstanceModelMatrix+2, 1); 
	glVertexAttribDivisor(shader->attributeInstanceModelMatrix+3, 1); 
	CheckGLError("RenderInstancingGroup::Render - glVertexAttribDivisor");

	if (debug == -5)
		return;

	// Bind textures?
	if (shader->uniformBaseTexture != -1)
	{
		// Bind texture if it isn't already bound.
		glActiveTexture(GL_TEXTURE0 + shader->diffuseMapIndex);		// Select server-side active texture unit
		// Bind texture
		glBindTexture(GL_TEXTURE_2D, reference->diffuseMap? reference->diffuseMap->glid : 0);
		/// Sets glTExParameter for Min/Mag filtering <- this needed every time?
		reference->diffuseMap->SetSamplingMode();
	}
	CheckGLError("RenderInstancingGroup::Render - reference->diffuseMap->SetSamplingMode");
	if (debug == -6)
		return;
				
	// for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4),
	// but faster.
	glDrawArraysInstanced(GL_TRIANGLES, 0, reference->model->mesh->vertexDataCount, currentItems);
	// Reset the instancing/divisor attributes or shading will fail on other shaders after this!
	CheckGLError("RenderInstancingGroup::Render - glDrawArraysInstanced");


	/// Reset divisors.
	glVertexAttribDivisor(shader->attributeInstanceModelMatrix, 0); 
	glVertexAttribDivisor(shader->attributeInstanceModelMatrix+1, 0); 
	glVertexAttribDivisor(shader->attributeInstanceModelMatrix+2, 0); 
	glVertexAttribDivisor(shader->attributeInstanceModelMatrix+3, 0); 

	// Unbind.
	graphicsState->BindVertexArrayBuffer(0);
	// Disable enabled vertex arrays.
	glDisableVertexAttribArray(shader->attributeInstanceModelMatrix);
	glDisableVertexAttribArray(shader->attributeInstanceModelMatrix+1);
	glDisableVertexAttribArray(shader->attributeInstanceModelMatrix+2);
	glDisableVertexAttribArray(shader->attributeInstanceModelMatrix+3);
	// Set the integer in the shader for toggling instanced rendering.
	glUniform1i(shader->uniformInstancingEnabled, 0);

	// Reset the instancing/divisor attributes or shading will fail on other shaders after this!
	CheckGLError("RenderInstancingGroup::Render");
}

