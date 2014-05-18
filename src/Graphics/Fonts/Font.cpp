#include "Font.h"

#include <iostream>
#include <fstream>

#include "Texture.h"
#include "TextureManager.h"
#include "Shader.h"
#include <String/Text.h>
#include <ctime>
#include <Timer/Timer.h>
#include "GraphicsState.h"
#include <cstring>

Timer renderTime;

String TextFont::defaultFontSource; // = "font3.png";

/// Prints the values of the error code in decimal as well as hex and the literal meaning of it.
extern void PrintGLError(const char * text);

TextFont::TextFont(){
	texture = NULL;
	color = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
	for (int i = 0; i < MAX_CHARS; ++i){
		charWidth[i] = 0.2f;
		charHeight[i] = 0.0f;
	}

	renderTime.Start();
	// Default space and tab to distances.
	charWidth[32] = 0.3f;	// Space
	charWidth[9] = 1.0f;	// Tab
	whitened = false;
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

	// WRite general font data first!
	f.write((char*)&this->fontWidth, sizeof(int));
	f.write((char*)&this->fontHeight, sizeof(int));

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
bool TextFont::Load(String path){
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

	// WRite general font data first!
	f.read((char*)&this->fontWidth, sizeof(int));
	f.read((char*)&this->fontHeight, sizeof(int));

	// Then character-specific data
	for (int i = 0; i < this->MAX_CHARS; ++i){
		// Width, height, stuff
		f.read((char*)&this->charWidth[i], sizeof(float));
		f.read((char*)&this->charHeight[i], sizeof(float));
	}

	f.close();
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
bool TextFont::LoadFromTexture(Texture * i_texture){
	if (i_texture == NULL){
		i_texture = TexMan.GetTextureByName(textureSource);
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
	fontWidth = texture->width / 16;
	fontHeight = texture->height / 16;

/*	int validPixels = 0;
	for (int i = 0; i < texture->width * texture->height; ++i){
		Vector4f pixel = texture->GetPixel(i);
		if (pixel.w == 0)
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
void TextFont::MakeTextureWhite(){
	float intensity = texture->GetMaxIntensity();
	Vector3f color(1,1,1);
	color *= 1 - intensity;
	texture->Add(color);
	whitened = true;
	/// Unbufferize it, so that it re-buffers!
//	TexMan.UnbufferizeTexture(texture);
//	this->texture->Colorize(Vector3f(1,1,1));
}

/// Calculates size of all ASCII characters in the texture (assumed standard 16x16 grid).
void TextFont::ParseTextureData(){
	// Parse texture data to determine the width of each character.
	for (int i = 0; i < 256; ++i){
		// For each character...
		bool firstPixel = true;
		int left, right, top, bottom;
		int startRow = 16 - i / 16;
		int startColumn = i % 16;
		int yStart = startRow * fontHeight - 1;
		int xStart = startColumn * fontWidth;
		// Look through all pixels in the specified tile.
		for (int y = yStart; y > yStart - fontHeight; --y){
			for (int x = xStart; x < xStart + fontWidth; ++x){
				// Check if valid pixel.
				Vector4f pixel = texture->GetPixel(x,y);
				if (pixel.w == 0)
					continue;
				if (firstPixel){
					left = right = x;
					top = bottom = y;
					firstPixel = false;
					continue;
				}
				if (x < left)
					left = x;
				if (x > right)
					right = x;
				if (y < top)
					top = y;
				if (y > bottom)
					bottom = y;
			}
		}
		if (firstPixel)
			continue;
		// Character evaluated.
		charWidth[i] = (right - left) / float(fontWidth);
		charHeight[i] = (bottom - top) / float(fontHeight);
		assert(charHeight[i] != 0);
		assert(charWidth[i] != 0);
		std::cout<<"\nCharacter "<<i<<": "<<(char)i<<" evaluated with width: "<<charWidth[i]<<" and height "<<charHeight[i];
	}
}

void TextFont::SetColor(Vector4f textColor){
	color = textColor;
}

/// Returns the size required by a call to RenderText if it were to be done now.
float TextFont::CalculateRenderSizeX(Text text, GraphicsState & graphics){
//	std::cout<<"\nTextFont::CalculateRenderSizeX";
    if (text.Length() < 1){
		// std::cout<<"\nERROR: NULL-string passed to Font::RenderText";
		return 0;
	}
	float sizeXRequired = 0;
	float rowSizeX = 0;

	const char * c_str = text.c_str();
	unsigned char c, previousChar;
	int characterX, characterY;
	float x1, x2, y1, y2;
	/// Starting position for each letter, starting at 0
	// xStart was previously "left", but is better suited as "centerX"
	float xStart = 0.0f;
	float yStart = 0.0f;
	float scale = 1.0f, halfScale = 0.5f;
	float spacing = scale * 0.15;
	xStart += spacing; // Always start with some space ;)
	for (int i = 0; i <= (int)strlen(c_str); ++i){
		c = c_str[i];
		// Handle special characters differently!
		switch(c){
			// Backspace, handle like in C++, allow for special escape sequences such as \n by looking at the next character.
			case '\\': 
			{ 
				char c2 = c_str[i+1];
				switch(c2){
					case 'n': // Newline!
						rowSizeX = xStart + charWidth[c] * halfScale;
						if (rowSizeX > sizeXRequired)
							sizeXRequired = rowSizeX;
						yStart -= scale;
						xStart = spacing;
						continue;
						break;
					// Unknown escape sequence, continue.
					default:
						break;
				}
				break;
			}
			case '\0': // Break the loop at the end o-o
                rowSizeX = xStart + charWidth[c] * halfScale;
                if (rowSizeX > sizeXRequired)
                    sizeXRequired = rowSizeX;
				break;
			case '\n': // And go to next row if we get a newline character!
                rowSizeX = xStart + charWidth[c] * halfScale;
                if (rowSizeX > sizeXRequired)
                    sizeXRequired = rowSizeX;
				yStart -= scale;
				xStart = spacing;
				continue;
				break;
			case '�':
				std::cout<<"ll;;;h";
				break;
		};

		xStart += charWidth[c] * halfScale;

		characterX = c % 16;
		characterY = c / 16;
		x1 = characterX / 16.0f;
		x2 = (characterX+1) / 16.0f;
		y1 = (16 - characterY) / 16.0f;
		y2 = (16 - characterY - 1) / 16.0f;

		// Move character in half it's width no matter what.
		// No need for letter spacing before first letter has passed by!
		xStart += charWidth[c] * halfScale + spacing;//i * scale * 0.6f;

		previousChar = c;
	}
	return sizeXRequired;
}

/// Renders text ^^
void TextFont::RenderText(Text & text, GraphicsState &graphicsState){
//	std::cout<<"\nTextFont::CalculateRenderSizeX";
	if (text.Length() < 1){
		// std::cout<<"\nERROR: NULL-string passed to Font::RenderText";
		return;
	}
	else if (!texture){
		std::cout<<"\nERROR: Texture not allocated in Font::RenderText";
		return;
	}
	if (!whitened){
		MakeTextureWhite();
	}

	/// Save old shader!
	Shader * oldShader = graphicsState.activeShader;

	// Enable textures if it wasn't already
	glEnable(GL_TEXTURE_2D);
	PrintGLError("Font.h: glEnable(GL_TEXTURE_2D) error");
	/// Set fill mode!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glUseProgram(0);

	glEnable(GL_TEXTURE_2D);
//	glEnable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	

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

	glDisable(GL_LIGHTING);


	// Disable depth test..
	glDisable(GL_DEPTH_TEST);

	if (texture->glid == -1)
		TexMan.BufferizeTexture(texture);
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

//	glUseProgram(graphicsState.activeShader->shaderProgram);

	const char * c_str = text.c_str();
	unsigned char c, previousChar;
	int characterX, characterY;
	float x1, x2, y1, y2;
	/// Starting position for each letter, starting at 0
	// xStart was previously "left", but is better suited as "centerX"
	float xStart = 0.0f;
	float yStart = 0.0f;
	float scale = 1.0f, halfScale = 0.5f;
	float spacing = scale * 0.15;
	xStart += spacing; // Always start with some space ;)
	/// Deprecated
	float bleedEdgeWidth = 0.00f;
	/// Begin the QUADS!
	glBegin(GL_QUADS);
//	std::cout<<"\nRendering text: "<<c_str;
	for (int i = 0; i <= (int)strlen(c_str); ++i){
		c = c_str[i];
    //    std::cout<<"\nc "<<i<<": (int)"<<(int)c<<" (char)"<<c;
		// Check for caret!
		if (text.caretPosition == i && renderTime.GetMs()% 1000 >= 500){
			char caret = '|';
			characterX = caret % 16;
			characterY = caret / 16;
			float offsetXStart = 0.0f;
			offsetXStart -= charWidth[caret]* halfScale;
			xStart += offsetXStart;
			x1 = characterX / 16.0f + bleedEdgeWidth;
			x2 = (characterX+1) / 16.0f - bleedEdgeWidth;
			y1 = (16 - characterY) / 16.0f - bleedEdgeWidth;
			y2 = (16 - characterY - 1) / 16.0f + bleedEdgeWidth;
			glTexCoord2f(x1, y2);
			glVertex3f(-halfScale + xStart, -scale + yStart, 0);
			glTexCoord2f(x2, y2);
			glVertex3f(halfScale + xStart, -scale + yStart, 0);
			glTexCoord2f(x2, y1);
			glVertex3f(halfScale + xStart, yStart, 0);
			glTexCoord2f(x1, y1);
			glVertex3f(-halfScale + xStart, yStart, 0);
			xStart -= offsetXStart;
		}

		// Handle special characters differently!
		switch(c){
			// Backspace, handle like in C++, allow for special escape sequences such as \n by looking at the next character.
			case '\\': 
			{ 
				char c2 = c_str[i+1];
				switch(c2){
					case 'n': // Newline!
						yStart -= scale;
						xStart = spacing;
						++i;
						continue;
						break;
					// Unknown escape sequence, continue.
					default:
						break;
				}
				break;
			}
			case '\0': // Break the loop at the end o-o
				break;
			case '\n': // And go to next row if we get a newline character!
				yStart -= scale;
				xStart = spacing;
				continue;
				break;
				
			case '�':
				std::cout<<"ll;;;h";
				break;
		};

		xStart += charWidth[c] * halfScale;

		characterX = c % 16;
		characterY = c / 16;
		x1 = characterX / 16.0f + bleedEdgeWidth;
		x2 = (characterX+1) / 16.0f - bleedEdgeWidth;
		y1 = (16 - characterY) / 16.0f - bleedEdgeWidth;
		y2 = (16 - characterY - 1) / 16.0f + bleedEdgeWidth;

	//	x1 = y1 = 0.0f;
	//	y2 = x2 = 1.0f;
	//	glBegin(GL_TRIANGLES);
		glTexCoord2f(x1, y2);
		glVertex3f(-halfScale + xStart, -scale + yStart, 0);
		glTexCoord2f(x2, y2);
		glVertex3f(halfScale + xStart, -scale + yStart, 0);
		glTexCoord2f(x2, y1);
		glVertex3f(halfScale + xStart, yStart, 0);
		glTexCoord2f(x1, y1);
		glVertex3f(-halfScale + xStart, yStart, 0);

		// Move character in half it's width no matter what.
		// No need for letter spacing before first letter has passed by!
		xStart += charWidth[c] * halfScale + spacing;//i * scale * 0.6f;


		previousChar = c;
	//	if (i == 0)
	//		break;
	}
	/// End the .. quads :D
	glEnd();

/*
glBegin(GL_TRIANGLES);
 for (int i = 0; i < strlen(text); ++i){
  char c = text[i];
  int x = c % 16;
  int y = c / 16;
  float x1 = x / 16.0f;
  float x2 = (x+1) / 16.0f;
  float y1 = (16 - y) / 16.0f;
  float y2 = (16 - y - 1) / 16.0f;
  float spacing = i * 2;
  glTexCoord2d(x1,y1); glVertex2d(-1 + spacing, -1);
  glTexCoord2d(x1,y2); glVertex2d(-1 + spacing, 1);
  glTexCoord2d(x2,y2); glVertex2d(1 + spacing, 1);

  glTexCoord2d(x2,y2); glVertex2d(1 + spacing, 1);
  glTexCoord2d(x2,y1); glVertex2d(1 + spacing, -1);
  glTexCoord2d(x1,y1); glVertex2d(-1 + spacing, -1);
 }
 glEnd();
*/

	PrintGLError("Font.h: Error after rendering error");

	/// Revert to old shader!
	graphicsState.activeShader = oldShader;
	if (oldShader == NULL)
		glUseProgram(0);
	else
		glUseProgram(oldShader->shaderProgram);
}
