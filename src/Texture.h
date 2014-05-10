/// Emil Hedemalm
/// 2014-02-17
/// General texture class, mostly based on PNG and RGBA types.

#ifndef TEXTURE_H
#define TEXTURE_H

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#include <GL/glew.h>
#include <cstdlib>
#include <Util.h>
#include "MathLib.h"

// For if-checking for applying normal maps, specular maps, etc.
#define DIFFUSE_MAP		0x0000001
#define SPECULAR_MAP	0x0000002
#define NORMAL_MAP		0x0000004
#define MAX_TEXTURE_TARGETS	NORMAL_MAP


struct TextureData{
};

/** A class for handling textures.
	A texture object is valid as long as it has a name (char *).
	The object need not necessarily be buffered to the graphics card (glid) in order to maintain
	a position in the TextureManager's array!
*/
class Texture{
public:
	Texture();
	~Texture();

	/// Resets width, height and creates a new data buffer after deleting the old one.
	void Resize(Vector2i newSize);

	// Flips along Y axis?
	void FlipY();
	// Flips along both X and Y axis.
	void FlipXY();

	/// Creates the data buffer. Width, height and bpp must be set before hand.
	bool CreateDataBuffer();

	/// Retrieves a sample color from the texture, using given amount of samples. Works in squares, meaning values 1, 4, 16, 64, etc. should be used.
	Vector4f GetSampleColor(int samples = 4);

	/// Uses glGetTexImage to procure image data from GL and insert it into the data-array of the texture object.
	void LoadDataFromGL();

	/// For debugging.
	bool MakeRed();
	/// Bufferizes into GL. Should only be called from the render-thread!
	bool Bufferize();

	/// Sets source of the texture.
	void SetSource(String str);
	/// Sets name of the texture.
	void SetName(String str);

	/// Gets pixel from indice.
	Vector4f GetPixel(int index);
	/// Gets color data from specified pixel in RGBA
	Vector4f GetPixel(int x, int y);
	/// Sets color of target pixel. Returns false if it is out of bounds.
	void SetPixel(int x, int y, Vector4f color);
	/// Pretty much highest result of: Vector3f(r,g,b).MaxPart() * a   -> perceived intensity on black background.
	float GetMaxIntensity();
	/// Linear addition of all rgb-compontents.
	void Add(Vector3f color, float alpha = 0.0f);
	/// Sets color for all pixels, not touching the alpha.
	void Colorize(Vector3f color);
	/// Setts color of all pixels.
	void SetColor(Vector4f color);

	/// Saves the texture in it's entirety to target file. If overwrite is false it will fail if the file already exists.
	bool Save(String toFile, bool overwrite = false);

	/// Returns the relative path of this texture's source.
	String RelativePath() const;

	/// Source file used to load the texture.
	String source;
	/// Abbreviated source name
	String name;

	/// For updating when painting in it.
	long long lastUpdate;
	
	enum formats{
		NULL_FORMAT,
		RGB,
		RGBA,
		CMYK,
		FORMATS
	};
	/** Format of inherent data, e.g: RGB, RGBA, CMYK, etc.
		Default is RGBA.
	*/
	int format;
	/** Bytes Per Pixel. Standard is 4 for RGBA, where each color has it's own byte representation (0-255).
	*/
	int bpp;

	/// Unique ID per session
	int ID() {return id;};
	/// OpenGL ID
	GLuint glid;

	/// Width in pixels
	int width;
	/// Height in pixels
	int height;
	/** Raw RGBA texture data.
		Pixel index psi below. and the pixel index with offsets of 0 to 3 give the RGBA components respectively.
		int psi = y * width * bpp + x * bpp;
			buf[psi] += color.x * 255.0f;
			buf[psi+1] += color.y * 255.0f;
			buf[psi+2] += color.z * 255.0f;
			buf[psi+3] += alpha * 255.0f;
		}
	*/
	unsigned char * data;
	/// Length of the above dynamic array.
	int dataBufferSize;
	/// Keeps track of active amount of users for this texture.
	int users;

	/// For using mipmaps. Default is true.
	bool mipmappingEnabled;
	/// Set this to true if you plan to bufferize this more than 1 time.
	bool dynamic;
	/// Set when rebufferization is queued. This flag is set for dynamic textures, for example for video produced by the MultimediaManager and its MultimediaStreams.
	bool queueRebufferization;
private:
	/// Private ID as given on creation
	int id;
	static int IDenumerator;

	// Consider including other relevant info if using this class for image manipulation, mipmapping or whatever.
};

#endif
