/// Emil Hedemalm
/// 2014-07-17 (lacked date earlier, at least from -12 if not earlier)
/// Class for handling custom fonts and rendering texts using them.

#ifndef FONT_H
#define FONT_H

class Texture;
class GraphicsState;
class Model;
class Shader;

#include "Util.h"
#include "MathLib.h"
#include "String/Text.h"

/** Returns false for all signs which are not rendered (or interacted with) per say. 
	I.e. \n, \r, \0, etc. will return false. \t and space (0x20) are both considered characters and will return true.
*/
bool IsCharacter(uchar c);

class TextFont {
public:
	TextFont ();
	~TextFont ();
	/** Returns the size required by a call to RenderText if it were to be done now. 
		In 'character width units'. Multiply this value with the model matrix and you 
		should get the rendered size (in world-space coordinates).
	*/
	Vector2f CalculateRenderSizeUnits(Text & text);
	Vector2f CalculateRenderSizePixels(Text & text, float textSizePixels);
	/// Calculates the render size in pixels if the text were to be rendered now.
	Vector2f CalculateRenderSizeWorldSpace(Text & text, GraphicsState & graphics, float textSizePixels);

	/// Renders text using matrices in the graphicsState, but with the default GL shader.
	void RenderText(
		Text & text,
		TextState textState,
		Color* overrideColor,
		GraphicsState & graphics,
		ConstVec3fr positionOffset,
		float textSizePixels
	);
	
	void SetTextureSource(const String s) { textureSource = s; };
	const String GetTextureSource() { return textureSource; };
	
	void SetColor(const Color& color);
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
	/// Source of the texture-file to be used.
	String source;

	static String defaultFontSource;
	static String defaultFontShader;

	static TextColors colors;

	//static Color idleColor;
	//static Color onHoverColor;
	//static Color onActiveColor;

	/// For extra colors or effects.
	bool hoveredOver;
	bool active;
	bool disabled; // For buttons currently disabled.
private:

	void SetTextSize(float textSizePixels);

	/// Sets up text-shader and font texture.
	bool PrepareForRender(GraphicsState& graphicsState, float textSizePixels);
	/// Renders the caret.
	void RenderCaret();
	// Renders character at current position.
	void RenderChar(
		uchar c,
		GraphicsState & graphicsState,
		ConstVec3fr positionOffset
	);
	/// For rendering a quad on top over the character's which are selected, pretty much.
	void RenderSelection(uchar currentChar);
	/// Re-instates old shader as needed.
	void OnEndRender(GraphicsState & graphicsState);

	/** New functions and variables which should make the calculation of render size and actual rendering of the 
		texts easier to understand, using an iterative state-machine approach.

		The flow to use the functions is as follows: 
		- Start wirth NewText()
		- Iterate StartChar, RenderChar and EndChar in that order until finished.
		RenderChar may be omitted, in which case only the size of the text will be calculated.
	*/
	/// Sets current text and clears old data. Prepares for a new parse or render.
	void NewText(Text & text, float textSizePixels);
	/// Evaluates current char. IF true, should skip processing hte current char in the rest of the procedure.
	bool EvaluateSpecialChar(float textSizePixels);
	// Moves variables to prepare for rendering this char.
	void StartChar(float textSizePixels);
	// Moves to the end of this character. 
	void EndChar(float textSizePixels);
	// Depends on the caret, offsets, etc.
	void RenderCaret(GraphicsState & graphicsState, ConstVec3fr positionOffset, float textSizePixels);
	/// New lines!
	void NewLine(float textSizePixels);
	/// Called when encountering the NULL-character.
	void EndText();

	
	/// Render options
	bool shaderBased; /// If true, use new shader-based rendering, instead of old glVertex, etc. Default true.
	Model * model; // Sprite XY model used to render the text.
	Shader * shader; // Current shader. Extraced in PrepareForRender

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
	unsigned char currentChar, lastChar, nextChar;
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
	Color color;
	Color* secondaryColor;

	/// Texture to be used when rendering.
	Texture * texture;
	/// Texture source o-o
	String textureSource;
	/// Width per character (in pixels) in the loaded texture
	int fontWidth;
	/// Height per character (in pixels) in the loaded texture
	int fontHeight;

	/// Ratio between the actual drawn letter height, and the total height allocated for this letter within the texture (inverse ratio of whitespace to edges to avoid letters sampling from one another).
	/// This should be a multiplier > 1.0, probably 1.2 or 2.0 depending on whitespace in your texture.
	float fontToWhitespaceScalingRatio;

	/// Index of character in current text which is rendered. Here to allow manipulation of it from within the various state-functions.
	int i;
};

#endif
