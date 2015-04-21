/// Emil Hedemalm
/// 2014-02-25
/// UI class to show images, often with restrictions if they should scale, tile, etc.

#include "UIImage.h"
#include "Shader.h"
#include "TextureManager.h"
#include "GraphicsState.h"
#include "UITypes.h"

#include "Mesh/Square.h"
#include "Graphics/OpenGL.h"

UIImage::UIImage(String nameAndTextureSource)
: UIElement()
{
	type = UIType::IMAGE;
	this->textureSource = nameAndTextureSource;
	name = FilePath::GetFileName(textureSource);
	color = Vector4f(1,1,1,1);

	navigationEnabled = true;
	editable = false;
}

UIImage::UIImage(String uiName, String textureSource)
: UIElement()
{
	type = UIType::IMAGE;
	name = uiName;
	this->textureSource = textureSource;
	color = Vector4f(1,1,1,1);
}


UIImage::~UIImage()
{
}

/// Subclassing in order to control rendering.
void UIImage::RenderSelf(GraphicsState & graphicsState)
{
	/// First render ourself using only black?
	// Depending on render-mode, render normally (stretched).
	if (false)
	{
		UIElement::RenderSelf(graphicsState);
		return;
	}
	
	/// Background stuff..?
	if (!isGeometryCreated)
	{
		AdjustToParent();
		CreateGeometry();
	}
	if (!isBuffered)
	{
		// Re-adjust to parent.
		AdjustToParent();
		ResizeGeometry();
		Bufferize();
	}

	/// Render our pictuuure.
	FetchBindAndBufferizeTexture();
	UpdateHighlightColor();

	Shader * shader = ActiveShader();

	// Bind vertices
	glBindBuffer(GL_ARRAY_BUFFER, vboBuffer);
	glVertexAttribPointer(shader->attributePosition, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, 0);		// Position
	
	// Bind UVs
	static const GLint offsetU = 6 * sizeof(GLfloat);		// Buffer already bound once at start!
	glVertexAttribPointer(shader->attributeUV, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void *)offsetU);		// UVs
	
    CheckGLError("GLError Binding Buffers");
	int vertices = vboVertexCount;



	// If moveable, translate it to it's proper position!
	if (moveable)
	{
		///
		if (shader->uniformModelMatrix != -1){
			/// TRanslatem power !
			Matrix4d * model = &graphicsState.modelMatrixD;
			float transX = alignmentX * parent->sizeX;
			float transY = alignmentY * parent->sizeY;
			model->Translate(transX,transY,0);
			graphicsState.modelMatrixF = graphicsState.modelMatrixD;
		}
	}

	/// Load in ze model matrix
	glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, graphicsState.modelMatrixF.getPointer());

    CheckGLError("GLError glUniformMatrix in UIElement");
	// Render normally
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, vboVertexCount);        // Draw All Of The Triangles At Once
	CheckGLError("GLError glDrawArrays in UIElement");

	// Unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	checkGLError();


/*

	/// Set mip-map filtering to closest
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/// NEW CODE

	/// Save old shader!
	Shader * oldShader = ActiveShader();

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

	ShadeMan.SetActiveShader(0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(graphicsState.projectionMatrixF.getPointer());
	Matrix4f modelView = graphicsState.viewMatrixF * graphicsState.modelMatrixF;
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelView.getPointer());
	glColor4f(color[0], color[1], color[2], color[3]);
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


	ShadeMan
	shader = oldShader;
	*/
}

// Creates the Square mesh used for rendering the UIElement and calls SetDimensions with it's given values.
void UIImage::CreateGeometry()
{
	Square * sq = new Square();
	this->mesh = sq;
	isGeometryCreated = true;
	// Resize.
	ResizeGeometry();
}

void UIImage::ResizeGeometry()
{
	if (!isGeometryCreated)
		CreateGeometry();
	assert(mesh);
	// o.o
	// std::cout<<"\nResizing geometry: L"<<left<<" R"<<right<<" B"<<bottom<<" T"<<top<<" Z"<<this->zDepth;
	
	// By default, demand dimensions to be proportional with the image.
	Vector2i size(right - left, top - bottom);
	if (!texture)
	{
		if (!FetchBindAndBufferizeTexture())
		{
			UIElement::ResizeGeometry();
			return;
		}
	}
	Vector2f texSize = texture->size;
	Vector2i center((right + left) / 2, (bottom + top) / 2);
	Vector2f elemSize(right - left, top - bottom);
	Vector2f relativeRatios = texSize / elemSize;
	// Choose the smaller ratio?
	bool xSmaller, yBigger;
	xSmaller = yBigger = relativeRatios.x < relativeRatios.y;
	float biggestPossible = relativeRatios.x > relativeRatios.y ? relativeRatios.x : relativeRatios.y;
	Vector2f biggestPossiblePx;
	if (biggestPossible > 1.f)
		biggestPossiblePx = texSize / biggestPossible;
	else if (biggestPossible > 0.f)
		biggestPossiblePx = texSize / biggestPossible;
	// Use largest possible ratio of the available space in the element.
	Vector2f modSize = biggestPossiblePx; 
	// Take into account shrinking too-large spaces.
	Vector2f halfModSize = modSize * 0.5;

	this->mesh->SetDimensions((float)center.x - halfModSize.x, (float)center.x + halfModSize.x, (float)center.y - halfModSize.y, (float)center.y + halfModSize.y, this->zDepth);
	for (int i = 0; i < children.Size(); ++i){
		children[i]->ResizeGeometry();
	}
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
