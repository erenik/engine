#ifndef FONT_H
#define FONT_H

/// Class for handling and rendering text!
class Texture;
class Text;
struct GraphicsState;

#include "Util.h"
#include "MathLib.h"

class TextFont {
public:
	TextFont ();
	~TextFont ();
	/// Returns the size required by a call to RenderText if it were to be done now. In... pixels? or units
	float CalculateRenderSizeX(Text text);
	/// Renders text using matrices in the graphicsState, but with the default GL shader.
	void RenderText(Text & text);
	void SetTextureSource(const String s) { textureSource = s; };
	const String GetTextureSource() { return textureSource; };
	void SetColor(Vector4f color);
	/** Saves the font-data to specified path.
		If the path length is 0 it will attempt to save to the same location as the given
		texture for the font.
	*/
	bool Save(String path = "");
	/** Loads font-data from specified path.
		If the path length is 0 it will attempt to load it from the same location as the
		given texture for the font.
	*/
	bool Load(String path = "");

	/// Pretty much the string before any texture info (like font1 for font1.png)
	String name;

	static String defaultFontSource;
private:
	/// If whitened for rendering.
	bool whitened;
	/// Loads texture for a font, calls ParseTextureData after the texture has been selected.
	bool LoadFromTexture(Texture * texture = NULL);
	/// Makes the texture-data white even if black? o-o
	void MakeTextureWhite();
	/// Calculates size of all ASCII characters in the texture (assumed standard 16x16 grid).
	void ParseTextureData();

	static const int MAX_CHARS = 256;
	/// Width and height of the character in question, from 0.0 to 1.0 of it's used space
	float charWidth[MAX_CHARS];
	float charHeight[MAX_CHARS];

	/// Color to be used when rendering the text.
	Vector4f color;
	/// Texture to be used when rendering.
	Texture * texture;
	/// Texture source o-o
	String textureSource;
	/// Width per character (in pixels) in the loaded texture
	int fontWidth;
	/// Height per character (in pixels) in the loaded texture
	int fontHeight;

};

#endif
