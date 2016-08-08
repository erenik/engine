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
GMBufferTexture::GMBufferTexture(Texture * texture) : GraphicsMessage(GM_BUFFER_TEXTURE) 
{
	assert(texture->source.Length());
	textureID = -1;
	t = texture;
	assert(t->BytesPerChannel() != 0);
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
}