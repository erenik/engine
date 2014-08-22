/// Emil Hedemalm
/// 2014-07-17 (lacked date earlier, at least from -12 if not earlier)
/// Class for handling custom fonts and rendering texts using them.

#ifndef FONT_H
#define FONT_H

class Texture;
class GraphicsState;

#include "Util.h"
#include "MathLib.h"
#include "String/Text.h"

/** Returns false for all signs which are not rendered (or interacted with) per say. 
	I.e. \n, \r, \0, etc. will return false. \t and space (0x20) are both considered characters and will return true.
*/
bool IsCharacter(char c);

class TextFont {
public:
	TextFont ();
	~TextFont ();
	/** Returns the size required by a call to RenderText if it were to be done now. 
		In 'character width units'. Multiply this value with the model matrix and you 
		should get the rendered size (in world-space coordinates).
	*/
	Vector2f CalculateRenderSizeUnits(Text text);
	/// Calculates the render size in pixels if the text were to be rendered now.
	Vector2f CalculateRenderSizeWorldSpace(Text text, GraphicsState * graphics);

	/// Renders text using matrices in the graphicsState, but with the default GL shader.
	void RenderText(Text & text, GraphicsState * graphics);
	
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

	/// Sets up text-shader and font texture.
	void PrepareForRender(GraphicsState * graphicsState);
	/// Renders the caret.
	void RenderCaret();
	// Renders character at current position.
	void RenderChar(char c);
	/// For rendering a quad on top over the character's which are selected, pretty much.
	void RenderSelection(char currentChar);
	/// Re-instates old shader as needed.
	void OnEndRender(GraphicsState * graphicsState);

	/** New functions and variables which should make the calculation of render size and actual rendering of the 
		texts easier to understand, using an iterative state-machine approach.

		The flow to use the functions is as follows: 
		- Start wirth NewText()
		- Iterate StartChar, RenderChar and EndChar in that order until finished.
		RenderChar may be omitted, in which case only the size of the text will be calculated.
	*/
	/// Sets current text and clears old data. Prepares for a new parse or render.
	void NewText(Text text);
	// Moves variables to prepare for rendering this char.
	void StartChar();
	// Moves to the end of this character. 
	void EndChar();
	/// New lines!
	void NewLine();
	/// Called when encountering the NULL-character.
	void EndText();

	/// Current text.
	Text currentText;
	/// Width of each row.
	List<float> rowSizes;

	/** Starting point for rendering, but also measuring. 0,0 at start, increase X-wise during the road. 
		Y will decrease by a certain amount each time the row changs (since rows go downward!)
	*/
	Vector2f pivotPoint;
	/// Scale and half-scale.
	Vector2f scale, halfScale;
	/// X: (extra) spacing between characters. Y: (extra) spacing between rows.
	Vector2f padding;
	/// Index of current character within the text.
	int currentCharIndex, lastCharIndex;
	char currentChar, lastChar, nextChar;
	/// Current row width.
	float rowSizeX;
	float maxRowSizeX;

	/// Is true, applies padding at the start and end of each row, as well as top and bottom.
	bool useFramePadding;


	/// OLD STUFF BELOW

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
