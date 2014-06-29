// Emil Hedemalm
// 2013-03-17

#include "GraphicsMessage.h"
#include "../GraphicsManager.h"
#include "TextureManager.h"
#include "GraphicsMessages.h"

GMBufferTexture::GMBufferTexture(int i_textureID) : GraphicsMessage(GM_BUFFER_TEXTURE) 
{
	textureID = i_textureID;
	t = NULL;
}
GMBufferTexture::GMBufferTexture(Texture * i_t) : GraphicsMessage(GM_BUFFER_TEXTURE) 
{
	textureID = -1;
	t = i_t;
}


void GMBufferTexture::Process()
{
	Texture * texturePtr;
	if (textureID != -1)
		texturePtr = TexMan.GetTextureByID(textureID);
	else
		texturePtr = t;

	if (texturePtr == NULL){
		std::cout<<"Null-Texture was queued to buffering!";
		return;
	}
	if (texturePtr->glid != -1 && !texturePtr->dynamic){
		std::cout<<"\nTexture "<<texturePtr->name<<" is already buffered (has glid)!";
		return;
	}

	texturePtr->Bufferize();
	return;

	std::cout<<"\nBuffering texture "<<texturePtr->name<<"...";
/*	
	glGenTextures(1, &tex->glid);
	glBindTexture(GL_TEXTURE_2D, tex->glid);	
	
	// Enable blending
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float z = -4;

	// Enable texturing
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex->glid);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->data.width, tex->data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->data.data);
*/

	GLuint error;

	// Enable blending
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// Buffer it again..
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, overlayTexture->data.width, overlayTexture->data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, overlayTexture->data.data);
	error = glGetError();
	if (error != GL_NO_ERROR){
		std::cout<<"\nGLError in GMBufferTexture "<<error;
	}

	// Generate texture
	glGenTextures(1, &texturePtr->glid);

	// Enable texturing
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texturePtr->glid);  // Bind glTexture ID.

	// Set texturing parameters for actively bound texture(!)
	// TODO: Implement mipmapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generate the texture Entity in GL	
	glTexImage2D(GL_TEXTURE_2D, 
		0, 
		GL_RGBA,
		texturePtr->width,		
		texturePtr->height,		
		0, 	
		GL_RGBA, 
		GL_UNSIGNED_BYTE, 
		texturePtr->data);

	error = glGetError();
	if (error != GL_NO_ERROR){
		std::cout<<"\nGL Error in GMBufferTexture: "<<error;
	}

	// Enable blending
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/// Deallocate data from memory if it is no longer needed?
	delete[] texturePtr->data;
	texturePtr->data = NULL;
}