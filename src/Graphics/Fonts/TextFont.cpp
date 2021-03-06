/// Emil Hedemalm
/// 2014-07-17 (lacked date earlier, at least from -12 if not earlier)
/// Class for handling custom fonts and rendering texts using them.

#include "TextFont.h"

#include <iostream>
#include <fstream>

#include "Texture.h"
#include "TextureManager.h"
#include "Graphics/Shader.h"
#include <String/Text.h>
#include <ctime>
#include <Timer/Timer.h>
#include "GraphicsState.h"
#include <cstring>

#include "Time/Time.h"

#include "Graphics/ShaderManager.h"

#include "File/LogFile.h"

#include "Model/Model.h"
#include "Model/ModelManager.h"

String TextFont::defaultFontSource = String("font3.png"); // = "font3.png";
String TextFont::defaultFontShader = String("Font");

/// Prints the values of the error code in decimal as well as hex and the literal meaning of it.
extern void PrintGLError(const char * text);

/** Returns false for all signs which are not rendered (or interacted with) per say. 
	I.e. \n, \r, \0, etc. will return false. \t and space (0x20) are both considered characters and will return true.
*/
bool IsCharacter(uchar c)
{
	switch(c)
	{
	case '\n':
	case '\r':
	case '\0':
		return false;
	}
	return true;
}

TextColors TextFont::colors = TextColors(Color::ColorByHexName("#AAAAAAff")
	, Color::ColorByHexName("#CCCCCCff")
	, Color::ColorByHexName("#FFFFFFFF"));

TextFont::TextFont()
{
	hoveredOver = false;
	active = false;

	texture = NULL;
	color = Color::defaultTextColor;
	for (int i = 0; i < MAX_CHARS; ++i){
		charWidth[i] = 0.2f;
		charHeight[i] = 0.0f;
	}

	// Default space and tab to distances.
	charWidth[32] = 0.3f;	// Space
	charWidth[9] = 1.0f;	// Tab
	whitened = false;
	useFramePadding = true;
	shaderBased = true;
	model = NULL;
	fontToWhitespaceScalingRatio = 1.0f;
};

TextFont::~TextFont(){
};

/** Saves the font-data to specified path.
	If the path length is 0 it will attempt to save to the same location as the given
	texture for the font.
*/
bool TextFont::Save(String path){
	if (path.Length() == 0){
		path = this->texture->source;
		path = path.Tokenize(".")[0]; // Cut off everything after the first decimal point, yes?
		path = path + ".font"; // Add other file-ending o-o
	}
	std::fstream f;
	f.open(path.c_str(), std::ios_base::out | std::ios_base::binary);
	assert(f.is_open());

	// Version!
	int version = 0; // Added fontToWhitespaceScalingRatio
	f.write((char*)&version, sizeof(int));

	// WRite general font data first!
	f.write((char*)&this->fontWidth, sizeof(int));
	f.write((char*)&this->fontHeight, sizeof(int));
	f.write((char*)&this->fontToWhitespaceScalingRatio, sizeof(float));

	// Then character-specific data
	for (int i = 0; i < this->MAX_CHARS; ++i){
		// Width, height, stuff
		f.write((char*)&this->charWidth[i], sizeof(float));
		f.write((char*)&this->charHeight[i], sizeof(float));
	}

	f.close();
	return true;
}

/** Loads font-data from specified path.
	If the path length is 0 it will attempt to load it from the same location as the
	given texture for the font.
*/
bool TextFont::Load(String path)
{
	this->source = path;
	std::fstream f;
	/// Check what kind of path we received first of all.
	if (path.Contains(".font")){
		/// Check to see if the file exists first!
		f.open(path.c_str(), std::ios_base::in | std::ios_base::binary);
		// If not successfully opened, abort!
		if (!f.is_open()){
			assert(f.is_open() && "Unable to open file stream to specified font file in Font::Load!");
			return false;
		}
	}
	// If we got a texture passed, check if a .font exists already!
	else if (path.Contains(".png")){
		textureSource = path;
		path = path.Tokenize(".")[0]; // Cut off everything after the first decimal point, yes?
		path = path + ".font"; // Add other file-ending o-o
		/// Check to see if the file exists first!
		f.open(path.c_str(), std::ios_base::in | std::ios_base::binary);
		// If not successfully opened, abort!
		if (!f.is_open()){
			path = path.Tokenize(".")[0] + ".png";
			return LoadFromTexture(TexMan.LoadTexture(path));
		}
	}
	// Create .font-version of path if possible
	else {
		path = textureSource;
		return this->LoadFromTexture();
	}

//	f.open(path.c_str(), std::ios_base::in | std::ios_base::binary);
	assert(f.is_open());

	int version;
	f.read((char*)&version, sizeof(int));
	if (version > 15) {
		path = textureSource;
		return this->LoadFromTexture();
	}

	// WRite general font data first!
	f.read((char*)&this->fontWidth, sizeof(int));
	f.read((char*)&this->fontHeight, sizeof(int));
	f.read((char*)&this->fontToWhitespaceScalingRatio, sizeof(float));

	// Then character-specific data
	for (int i = 0; i < this->MAX_CHARS; ++i){
		// Width, height, stuff
		f.read((char*)&this->charWidth[i], sizeof(float));
		f.read((char*)&this->charHeight[i], sizeof(float));
	}

	f.close();

	// Invisible characters have no width!
	charWidth['\n'] = 0;
	charWidth['\0'] = 0;

	texture = TexMan.LoadTexture(textureSource);

	/// Set name accordingly.
	if (texture){
		List<String> tokens = texture->source.Tokenize("/\\");
		name = tokens[tokens.Size()-1];
		/// Remove extension, if any.
		if (name.Contains("."))
			name = name.Tokenize(".")[0];
	}
	return true;
}



/// Loads texture for a font, calls ParseTextureData after the texture has been selected.
bool TextFont::LoadFromTexture(Texture * i_texture)
{
	if (i_texture == NULL){
		i_texture = TexMan.GetTexture(textureSource);
		if (i_texture == NULL)
			return false;
	}

	if (!i_texture){
		std::cout<<"\nERROR: NULL-texture passed to Font::SetTexture";
		return false;
	}
	/// Remove earlier path stuff.
	List<String> tokens = i_texture->source.Tokenize("/\\");
	name = tokens[tokens.Size()-1];
	/// Remove extension, if any.
	if (name.Contains("."))
		name = name.Tokenize(".")[0];

	texture = i_texture;
	fontWidth = texture->size.x / 16;
	fontHeight = texture->size.y / 16;

/*	int validPixels = 0;
	for (int i = 0; i < texture->width * texture->height; ++i){
		Vector4f pixel = texture->GetPixel(i);
		if (pixel[3] == 0)
			continue;
		++validPixels;
	}
	assert(validPixels > 256);
*/
	MakeTextureWhite();
	ParseTextureData();

	// Save automatically after parsing unless specified otherwise!
	Save();
	return true;
};

/// Makes the texture-data white even if black? o-o
void TextFont::MakeTextureWhite()
{
	float intensity = texture->GetMaxIntensity();
	Vector3f color(1,1,1);
	color *= 1 - intensity;
	texture->Add(color);
	whitened = true;
}

/// Calculates size of all ASCII characters in the texture (assumed standard 16x16 grid).
void TextFont::ParseTextureData(){
	// Parse texture data to determine the width of each character.
	int MAX_CHARS = 256;
	for (int i = 0; i < MAX_CHARS; ++i){
		// For each character...
		bool firstPixel = true;
		float left, right, top, bottom;
		int startRow = 16 - i / 16;
		int startColumn = i % 16;
		int yStart = startRow * fontHeight - 1;
		int xStart = startColumn * fontWidth;
		// Look through all pixels in the specified tile.
		for (int y = yStart; y > yStart - fontHeight; --y){
			for (int x = xStart; x < xStart + fontWidth; ++x){
				// Check if valid pixel.
				Vector4f pixel = texture->GetPixel(x,y);
				if (pixel[3] == 0)
					continue;
				if (firstPixel){
					left = x - 0.5f;
					right = x + 0.5f;
					top = y - 0.5f;
					bottom = y + 0.5f;
					firstPixel = false;
					continue;
				}
				if (x < left)
					left = x - 0.5f;
				if (x > right)
					right = x + 0.5f;
				if (y < top)
					top = y - 0.5f;
				if (y > bottom)
					bottom = y + 0.5f;
			}
		}
		if (firstPixel)
			continue;
		// Character evaluated.
		charWidth[i] = (right - left) / float(fontWidth);
		charHeight[i] = (bottom - top) / float(fontHeight);
		if (charHeight[i] == 0)
		{
			LogGraphics("Font char "+String(i)+" height 0. Bottom: "+String(bottom)+" Top: "+String(top)+". Setting default height of 1.0", WARNING);
			charHeight[i] = 1.0f;
		}
		assert(charHeight[i] != 0);
		assert(charWidth[i] != 0);
		std::cout<<"\nCharacter "<<i<<": "<<(char)i<<" evaluated with width: "<<charWidth[i]<<" and height "<<charHeight[i];
	}
	// Capital 'A' to calculate font to whitespace scaling ratio.
	fontToWhitespaceScalingRatio = 1 / charHeight[65];
	// Then..! Re-calculate the width and heigh of each character using the given factor.
	// This to make it so that height is most often 1.0, and width varying in order to render them in a more unified way.
	for (int i = 0; i < MAX_CHARS; ++i) {
		charWidth[i] = charWidth[i] * fontToWhitespaceScalingRatio;
		charHeight[i] = charHeight[i] * fontToWhitespaceScalingRatio;
	}
}

void TextFont::SetColor(const Color& textColor){
	color = textColor;
}

/** New functions and variables which should make the calculation of render size and actual rendering of the 
	texts easier to understand, using an iterative state-machine approach.
*/
/// Sets current text and clears old data. Prepares for a new parse or render.
void TextFont::NewText(Text & text, float textSizePixels)
{
	this->currentText = text;
	rowSizes.Clear();

	scale = Vector2f(1,1);
	halfScale = scale * 0.5f;
	padding = scale * 0.15f;
	padding[1] *= 0.5f;
	maxRowSizeX = 0;

	pivotPoint = Vector2f(0, -halfScale[1] * textSizePixels);
	// If using frame-padding...
	if (useFramePadding)
		pivotPoint += Vector2f(padding[0] * textSizePixels, -padding[1] * textSizePixels);
}

/// Evaluates current char. IF true, should skip processing hte current char in the rest of the procedure.
bool TextFont::EvaluateSpecialChar(float textSizePixels)
{
	// Evaluate special-characters!
	switch(currentChar)
	{
		// Backspace, handle like in C++, allow for special escape sequences such as \n by looking at the next character.
		case '\\': 
		{ 
			switch(nextChar)
			{
				case 'n': // Newline!
					NewLine(textSizePixels);
					// Increment by two, so the \n isn't actually rendered?
					++i;
					return true;
				// Unknown escape sequence, continue.
				default:
					break;
			}
			break;
		}
		case '\n': // And go to next row if we get a newline character!
			NewLine(textSizePixels);
			return true;
		case '�':
			std::cout<<"ll;;;h";
			break;
	};
	return false;
}

void TextFont::StartChar(float textSizePixels)
{
	pivotPoint[0] += charWidth[currentChar] * halfScale[0] * textSizePixels;
}

void TextFont::EndChar(float textSizePixels)
{
	float characterSpacing = 0.05f;
	pivotPoint[0] += (charWidth[currentChar] * halfScale[0] + characterSpacing) * textSizePixels;
	// Row finished?
	if (IsCharacter(nextChar))
		pivotPoint[0] += padding[0] * textSizePixels;
}

/// New lines!
void TextFont::NewLine(float textSizePixels)
{
	// Add padding if needed
	if (useFramePadding)
		pivotPoint[0] += padding[0] * textSizePixels;
	// Save row size.
	rowSizeX = pivotPoint[0];
	if (rowSizeX > maxRowSizeX)
		maxRowSizeX = rowSizeX;
	rowSizes.Add(rowSizeX);
	// Go left to the start of the row..
	pivotPoint[0] = 0;
	if (useFramePadding)
		pivotPoint[0] = padding[0] * textSizePixels;
	// And go down one row.
	float lineSpacing = 1.15f;
	pivotPoint[1] -= lineSpacing * textSizePixels;
}

/// Called when encountering the NULL-character.
void TextFont::EndText()
{
	// Add padding if needed
	if (useFramePadding)
		pivotPoint[0] += padding[0];
	// Save row size.
	rowSizeX = pivotPoint[0];
	if (rowSizeX > maxRowSizeX)
		maxRowSizeX = rowSizeX;
	rowSizes.Add(rowSizeX);

}


/// Returns the size required by a call to RenderText if it were to be done now.
Vector2f TextFont::CalculateRenderSizeUnits(Text & text)
{
	if (text.Length() < 1)
	{
		return Vector2f(scale[0], scale[1]);
	}
	// Set starting variables.
	NewText(text, 1);

	// Go up to and include the NULL-sign!
	for (int i = 0; i < text.ArraySize(); ++i)
	{
		currentCharIndex = i;
		currentChar = text.c_str()[i];
		if (currentChar == '\0')
		{
			EndText();
			break;
		}
		nextChar = text.c_str()[i + 1];
		if (EvaluateSpecialChar(1))
			continue;
		StartChar(1);
		// RenderChar();
		EndChar(1);
		lastChar = currentChar;
	}
	return Vector2f (maxRowSizeX, AbsoluteValue(pivotPoint.y));
}

Vector2f TextFont::CalculateRenderSizePixels(Text & text, float textSizePixels) {
	return CalculateRenderSizeUnits(text) * textSizePixels;
}

/// Calculates the render size in pixels if the text were to be rendered now.
Vector2f TextFont::CalculateRenderSizeWorldSpace(Text & text, GraphicsState & graphics, float textSizePixels)
{
	// Just grab required render size and multiply with the model-matrix?
	Vector2f renderSize = CalculateRenderSizePixels(text, textSizePixels);
	Vector4f size(renderSize[0], renderSize[1], 0, 0);
	Vector4f transformed = graphics.modelMatrixF.Product(size);
	return Vector2f(transformed[0], AbsoluteValue(transformed[1]));
}



/// Renders text ^^
void TextFont::RenderText(
	Text & text,
	TextState textState,
	Color* overrideColor,
	GraphicsState & graphicsState,
	ConstVec3fr positionOffset,
	float textSizePixels
) {
	// No point wasting cycles on empty texts.
	//if (text.Length() == 0)
	//	return;

	Matrix4f modelMatrixF = graphicsState.modelMatrixF;

	// Set starting variables.
	NewText(text, textSizePixels);

	auto * colorsToUse = &colors;
	if (text.colors)
		colorsToUse = text.colors;

	/// One color for all text?
	auto color = colorsToUse->Get(textState);
	if (overrideColor != nullptr)
		color = *overrideColor;

	this->SetColor(color);
	
	/// Save old shader!
	Shader * oldShader = ActiveShader();
	// Load shader, set default uniform values, etc.
	if (!PrepareForRender(graphicsState, textSizePixels))
		return;

	/// Sort the carets in order to render selected text properly.
	int min, max;
	if (text.caretPosition < text.previousCaretPosition)
	{
		min = text.caretPosition;
		max = text.previousCaretPosition;
	}
	else 
	{
		max = text.caretPosition;
		min = text.previousCaretPosition;
	}

	bool shouldRenderCaret = Timer::GetCurrentTimeMs() % 1000 > 500;
	Vector3f renderCaretPivot;
	if (text.Length() == 0 && shouldRenderCaret)
		renderCaretPivot = pivotPoint;
	for (i = 0; i < text.Length(); ++i)
	{
		if (text.caretPosition == i && shouldRenderCaret)
		{
			renderCaretPivot = pivotPoint;
		}
		currentCharIndex = i;
		currentChar = text.c_str()[i];
		if (currentChar == 0)
			break;
		nextChar = text.c_str()[i + 1];

		if (EvaluateSpecialChar(textSizePixels))
			continue;

		StartChar(textSizePixels);				// Move in.
		RenderChar(currentChar, graphicsState, positionOffset);	// Render
		/// If we are between the 2 active carets, render the region the char covers over with a white quad ?
		if (text.previousCaretPosition != -1 && i >= min && i < max)
		{
			RenderSelection(currentChar);			
		}
		EndChar(textSizePixels);					// Move out.
		lastChar = currentChar;
	}

	// For texts with contents.
	if (text.caretPosition >= 0)
	{
		if (shouldRenderCaret) {
			if (text.Length() == 0)
				renderCaretPivot = Vector2i(5, -textSizePixels * 0.5f);
			else if (renderCaretPivot.MaxPart() != 0)
				pivotPoint = renderCaretPivot;
			RenderCaret(graphicsState, positionOffset, textSizePixels);
		}
	}

	OnEndRender(graphicsState);
	/// Revert to old shader!
	ShadeMan.SetActiveShader(&graphicsState, oldShader);

	graphicsState.modelMatrixF = modelMatrixF;
}


void TextFont::RenderCaret(GraphicsState & graphicsState, ConstVec3fr positionOffset, float textSizePixels) {
	char fontCaret = '|';
	// Random offset based on char width..?
	float moveDistance = 0;
	pivotPoint[0] += moveDistance; // Move in and back.
	SetTextSize(textSizePixels * 1.20f);
	// Set transparency for caret rendering.
	glUniform4f(shader->uniformPrimaryColorVec4, color.x, color.y, color.z, color.w * 0.5f);

	RenderChar(fontCaret, graphicsState, positionOffset);

	// Revert back pivot point
	pivotPoint[0] -= moveDistance;
}


void TextFont::SetTextSize(float textSizePixels) {
	glUniform1f(shader->uniformScale, textSizePixels * fontToWhitespaceScalingRatio);
}

bool TextFont::PrepareForRender(GraphicsState & graphicsState, float textSizePixels)
{
	if (!texture){
		std::cout<<"\nERROR: Texture not allocated in Font::RenderText";
		return false;
	}
	if (!whitened && false){ // Change so whitening process is executed only for those fonts which need it.
		MakeTextureWhite();
	}
	/// Bufferize texture as needed.
	if (texture->glid == -1)
	{
		texture->releaseOnBufferization = false;
		TexMan.BufferizeTexture(texture);
	}

	/// Prepare shader.
	if (shaderBased)
	{
		// Load shader.
		shader = ShadeMan.SetActiveShader(&graphicsState, graphicsState.fontShaderName); // "Font" by default
		if (!shader)
			return false;
		// Enable texture
		glEnable(GL_TEXTURE_2D);
		// Set matrices.
		shader->SetProjectionMatrix(graphicsState.projectionMatrixF);
		shader->SetViewMatrix(graphicsState.viewMatrixF);
		SetTextSize(textSizePixels);
		shader->SetModelMatrix(Matrix4f::Identity());
		Matrix4f modelMatrix = graphicsState.modelMatrixF;
		//shader->SetModelMatrix(graphicsState.modelMatrixF);
		// Set text color
		auto vec = color;
		glUniform4f(shader->uniformPrimaryColorVec4, vec.x, vec.y, vec.z, vec.w);
		
		// set hover state
		glUniform1i(shader->uniformHoveredOver, hoveredOver? 1 : 0);

		// when texture area is small, bilinear filter the closest mipmap
		// when texture area is large, bilinear filter the original
		bool texture_sampling_nearest = true;
		if (texture_sampling_nearest) {
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else {
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		// Set some color.. hmm.
		// 		glColor4f(color[0], color[1], color[2], color[3]);
				glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// Don't write depth to depth buffer.
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);

		// Set texture in shader? hm.
		glBindTexture(GL_TEXTURE_2D, texture->glid);

		// Fetch and bufferizes model.
		if (!model)
			model = ModelMan.GetModel("sprite.obj");
		model->BufferizeIfNeeded();
	}
	else 
	{
		// Enable textures if it wasn't already
		glEnable(GL_TEXTURE_2D);
		PrintGLError("Font.h: glEnable(GL_TEXTURE_2D) error");
		/// Set fill mode!
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		ShadeMan.SetActiveShader(&graphicsState, nullptr);

		glEnable(GL_TEXTURE_2D);
	//	glEnable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glLoadMatrixf(graphicsState.projectionMatrixF.getPointer());
		Matrix4f modelView = graphicsState.viewMatrixF * graphicsState.modelMatrixF;
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(modelView.getPointer());
		auto vec = color;
		glColor4f(vec.x, vec.y, vec.z, vec.w);
		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDisable(GL_LIGHTING);

		// Disable depth test.. lol no?
	//	glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		glBindTexture(GL_TEXTURE_2D, texture->glid);

				// when texture area is small, bilinear filter the closest mipmap
		// when texture area is large, bilinear filter the original
		bool linear = false;
		if (linear){
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else {
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}


	//	glUseProgram(shader->shaderProgram);
		/// Begin the QUADS!
		glBegin(GL_QUADS);
	}

	return true;
}

// Renders character at current position.
void TextFont::RenderChar(uchar c, GraphicsState& graphicsState, ConstVec3fr positionOffset)
{
	float & xStart = pivotPoint[0];
	float & yStart = pivotPoint[1];


	// Render an actual character.
	if (shaderBased)
	{
		Vector2f position = pivotPoint + positionOffset;

		// Set location via uniform?
		int character = c;
		glUniform1i(shader->uniformCharacter, character);
		glUniform2f(shader->uniformPivot, position.x, position.y);
		// Render it.
		model->Render(&graphicsState);
	}
	/// Old render
	else 
	{
		int characterX = c % 16;
		int characterY = c / 16;
		/// Texture co-ordinates.
		float x1,x2,y1,y2;
		x1 = characterX / 16.0f;
		x2 = (characterX+1) / 16.0f;
		y1 = (16 - characterY) / 16.0f;
		y2 = (16 - characterY - 1) / 16.0f;
		// And actual rendering.
		glTexCoord2f(x1, y2);
		glVertex3f(-halfScale[0] + xStart, -halfScale[1] + yStart, 0);
		glTexCoord2f(x2, y2);
		glVertex3f(halfScale[0] + xStart, -halfScale[1] + yStart, 0);
		glTexCoord2f(x2, y1);
		glVertex3f(halfScale[0] + xStart, halfScale[1] + yStart, 0);
		glTexCoord2f(x1, y1);
		glVertex3f(-halfScale[0] + xStart, halfScale[1] + yStart, 0);
	}
}

/// Render the character as selected.
void TextFont::RenderSelection(uchar c)
{
	/// No good solution for this... yet.
	if (shaderBased)
	{
		return;
	}

	// Stop the quads.
	glEnd();

	CheckGLError("TextFont::RenderSelection 1");

	// Switch to additive rendering 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	

	CheckGLError("TextFont::RenderSelection 1.5");

	int characterX = c % 16;
	int characterY = c / 16;
	/// Texture co-ordinates.
	float & xStart = pivotPoint[0];
	float & yStart = pivotPoint[1];
	
	// Bind the texture to use. Preferably a gray sort.
	Texture * tex = TexMan.GetTextureByHex32(0xFFFFFF22);
	if (tex->glid == -1)
		tex->Bufferize();
	glBindTexture(GL_TEXTURE_2D, tex->glid);

	CheckGLError("TextFont::RenderSelection 2");

	float width = this->charWidth[c];
	float halfWidth = width * 0.5f;

	// Render the quad!
	glBegin(GL_QUADS);

	float x1 = halfWidth * -halfScale[0] + xStart - padding[0] * 0.5f,
		x2 = halfWidth * halfScale[0] + xStart + padding[0] * 0.5f;

	// And actual rendering.
	glTexCoord2f(0, 1);
	glVertex3f(x2, -halfScale[1] + yStart, 0);
	glTexCoord2f(1, 1);
	glVertex3f(x1, -halfScale[1] + yStart, 0);
	glTexCoord2f(1, 0);
	glVertex3f(x1, halfScale[1] + yStart, 0);
	glTexCoord2f(0, 0);
	glVertex3f(x2, halfScale[1] + yStart, 0);

	glEnd();

	CheckGLError("TextFont::RenderSelection 3");

	// Re-bind old texture.
	glBindTexture(GL_TEXTURE_2D, this->texture->glid);

	// Reset blend-mode.
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	CheckGLError("TextFont::RenderSelection 4");
	// Begin quads again.
	glBegin(GL_QUADS);
}

	

/// Re-instates old shader as needed.
void TextFont::OnEndRender(GraphicsState & graphicsState)
{
	if (shaderBased)
	{
		// Bind 0?
	}
	else 
	{
		glEnd();
	}
	/// enable writing to depth-buffer again.4
	glDepthMask(GL_TRUE);

	PrintGLError("Font.h: Error after rendering error");

}

