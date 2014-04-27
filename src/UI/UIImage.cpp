/// Emil Hedemalm
/// 2014-02-25
/// UI class to show images, often with restrictions if they should scale, tile, etc.

#include "UIImage.h"
#include "Shader.h"
#include "TextureManager.h"
#include "GraphicsState.h"
#include "UITypes.h"

UIImage::UIImage(String textureSource)
: UIElement()
{
	type = UIType::IMAGE;
	this->textureSource = textureSource;
	name = FilePath::GetFileName(textureSource);
	color = Vector4f(1,1,1,1);
}

UIImage::~UIImage()
{
}

/// Subclassing in order to control rendering.
void UIImage::RenderSelf(GraphicsState & graphicsState)
{
	/// First render ourself using only black?
	UIElement::RenderSelf(graphicsState);

	return;
	/// Render our pictuuure.

	/// Set mip-map filtering to closest
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/// NEW CODE

	/// Save old shader!
	Shader * oldShader = graphicsState.activeShader;

	// Enable textures if it wasn't already
	glEnable(GL_TEXTURE_2D);
	/// Set fill mode!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	/// Rebuffer the texture as needed.
	if (!texture)
	{
		texture = TexMan.GetTexture(textureSource);
		if (!texture)
			return;
	}
	if (texture->glid == -1)
		texture->Bufferize();

	glUseProgram(0);
	graphicsState.activeShader = NULL;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(graphicsState.projectionMatrixF.getPointer());
	Matrix4f modelView = graphicsState.viewMatrixF * graphicsState.modelMatrixF;
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelView.getPointer());
	glColor4f(color.x, color.y, color.z, color.w);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/// Bind it again after switching program.
	glBindTexture(GL_TEXTURE_2D, texture->glid);

	/// Disable depth, test, since I'm lazy.
	glDisable(GL_DEPTH_TEST);

	float imageWidth = texture->width;
	float imageHeight = texture->height;

	float aspectRatio = imageWidth / imageHeight;
	float videoLeft, videoRight, videoTop, videoBottom;

	float uiElementAspectRatio = ((float)sizeX) / sizeY;

	float centerX = (right + left) / 2.0f;
	float centerY = (top + bottom) / 2.0f;
	float halfSizeX = sizeX / 2.0f;
	float halfSizeY = sizeY / 2.0f;

	float ratioDiff = aspectRatio / uiElementAspectRatio;
	if (ratioDiff > 1.0f)
	{
		halfSizeY /= ratioDiff;
	}
	else if (ratioDiff < 1.0f) {
		halfSizeX *= ratioDiff;
	}


	/// Render a quad.
	float x1 = centerX - halfSizeX,
		x2 = centerX + halfSizeX,
		y1 = centerY + halfSizeY,
		y2 = centerY - halfSizeY;

	float texCoordX1 = 0, texCoordX2 = 1,
		texCoordY1 = 1, texCoordY2 = 0;

	glBegin(GL_QUADS);
		glTexCoord2f(texCoordX1, texCoordY1);
		glVertex3f(x1, y1, 0);
		glTexCoord2f(texCoordX2, texCoordY1);
		glVertex3f(x2, y1, 0);
		glTexCoord2f(texCoordX2, texCoordY2);
		glVertex3f(x2, y2, 0);
		glTexCoord2f(texCoordX1, texCoordY2);
		glVertex3f(x1, y2, 0);
	glEnd();

	glUseProgram(oldShader->shaderProgram);

	graphicsState.activeShader = oldShader;

}

Texture * UIImage::GetTexture()
{
	return texture;
}

String UIImage::GetTextureSource()
{
	return textureSource;
}


void UIImage::SetTextureSource(String src)
{
	textureSource = src;
	texture = NULL;
}
