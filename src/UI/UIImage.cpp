/// Emil Hedemalm
/// 2014-02-25
/// UI class to show images, often with restrictions if they should scale, tile, etc.

#include "UIImage.h"
#include "Graphics/Shader.h"
#include "TextureManager.h"
#include "GraphicsState.h"
#include "UITypes.h"

#include "Mesh/Square.h"
#include "Graphics/OpenGL.h"

UIImage::UIImage(String nameAndTextureSource)
: UIElement()
{
	type = UIType::IMAGE;
	this->visuals.textureSource = nameAndTextureSource;
	name = FilePath::GetFileName(visuals.textureSource);
	visuals.color = Vector4f(1,1,1,1);

	navigationEnabled = true;
	editable = false;
}

UIImage::UIImage(String uiName, String textureSource)
: UIElement()
{
	type = UIType::IMAGE;
	name = uiName;
	visuals.textureSource = textureSource;
	visuals.color = Vector4f(1,1,1,1);
}


UIImage::~UIImage()
{
}

/// Subclassing in order to control rendering.
void UIImage::RenderSelf(GraphicsState & graphicsState)
{
	return UIElement::RenderSelf(graphicsState);
}

// Creates the Square mesh used for rendering the UIElement and calls SetDimensions with it's given values.
void UIImage::CreateGeometry(GraphicsState* graphicsState)
{
	visuals.CreateGeometry(graphicsState, layout);
	/*
	Square * sq = new Square();
	this->visuals.mesh = sq;
	visuals.isGeometryCreated = true;
	*/
	// Resize.
	ResizeGeometry(graphicsState);
}

void UIImage::ResizeGeometry(GraphicsState* graphicsState)
{
	if (!IsGeometryCreated())
		CreateGeometry(graphicsState);

	visuals.ResizeGeometry(graphicsState, layout);
	/*
	assert(mesh);
	// o.o
	// std::cout<<"\nResizing geometry: L"<<left<<" R"<<right<<" B"<<bottom<<" T"<<top<<" Z"<<this->zDepth;
	
	// By default, demand dimensions to be proportional with the image.
	Vector2i size(right - left, top - bottom);
	if (!texture)
	{
		if (!FetchBindAndBufferizeTexture())
		{
			UIElement::ResizeGeometry(graphicsState);
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
	// Mark as not buffered to refresh it properly
	isBuffered = false;
	*/

	for (int i = 0; i < children.Size(); ++i) {
		children[i]->ResizeGeometry(graphicsState);
	}
}

/// Called after FetchBindAndBufferizeTexture is called successfully. (may also be called other times).
void UIImage::OnTextureUpdated(GraphicsState* graphicsState)
{
	ResizeGeometry(graphicsState);
}



Texture * UIImage::GetTexture()
{
	return visuals.texture;
}

String UIImage::GetTextureSource()
{
	return visuals.textureSource;
}


void UIImage::SetTextureSource(String src)
{
	visuals.textureSource = src;
	visuals.texture = NULL;
}
