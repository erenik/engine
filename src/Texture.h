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
#include "DataTypes.h"

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
class Texture
{
	// Require usage of the texture manager to allocate textures.
	friend class TextureManager;
	Texture();
public:
	~Texture();

	/// Deallocates the memory for the data buffers located on the heap.
	void Deallocate();
	// Reallocate based on new size and format.
	void Reallocate();


	// Same thing as Resize.
	void SetSize(Vector2i newSize);
	/// Resets width, height and creates a new data buffer after deleting the old one. Returns false if it failed (due to lacking memory).
	bool Resize(Vector2i newSize);

	// Flips along Y axis?
	void FlipY();
	// Flips along both X and Y axis.
	void FlipXY();

	/// Creates the data buffer. Width, height and bpp must be set before hand.
	bool CreateDataBuffer();

	/// Retrieves a sample color from the texture, using given amount of samples. Works in squares, meaning values 1, 4, 16, 64, etc. should be used.
	Vector4f GetSampleColor(int samples = 4);

	/// 0 - Unsigned char, 1 - Int, 2 - Float
	int DataType();
	int NumPixels();
	/// Returns amount of channels, depending on the format.
	int GetChannels();
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

	/// Sets color of target pixel. Pixel size in pixels x pixels.
	void SetPixel(Vector2i location, Vector4f color, int pixelSize = 1);
	/// Sets color of target pixel. 
	void SetPixel(int x, int y, Vector4f color);
	/// Pretty much highest result of: Vector3f(r,g,b).MaxPart() * a   -> perceived intensity on black background.
	float GetMaxIntensity();
	/// Linear addition of all rgb-compontents.
	void Add(Vector3f color, float alpha = 0.0f);
	/// Sets color for all pixels, not touching the alpha.
	void Colorize(Vector3f color);
	/// Setts color of all pixels.
	void SetColor(Vector4f color);
	void SetColorOfColumn(int column, Vector4f color);
	void SetColorOfRow(int row, Vector4f color);

	/// Sets sampling mode based on samplingMode current value.
	void SetSamplingMode();

	/// Saves the texture in it's entirety to target file. If overwrite is false it will fail if the file already exists.
	bool Save(String toFile, bool overwrite = false);

	/// Returns the relative path of this texture's source.
	String RelativePath() const;

	/// Source file used to load the texture.
	String source;
	/// Abbreviated source name
	String name;


	/// Is set automatically upon creation to the current millisecond count.
	int64 creationDate;
	/// For updating when painting in it.
	long long lastUpdate;
	
	enum formats{
		NULL_FORMAT,
		SINGLE_16F, // Single channel red/greyscale image, 16 bits float per pixel.
		RGB,
		RGB_8 = RGB, // standard 8 bit per channel, 3 channels
		RGB_16F, // 3 channel, 16 bit float per channel. 
		RGB_32F,
		RGBA, // standard 8 bit per channel, 4 channels
		RGBA_8 = RGBA,
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

	/// Width and height. Might replace width and height above with this one?
	Vector2i size;

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

	/// Pointers to access the data. If the data is allocated of type char, cData should point to data and fData to NULL.
	unsigned char * cData;
	/// Pointers to access the data. If the data is allocated of type float, fData should point to data and cData to NULL.
	float * fData;

	/// Length of the above dynamic array.
	int dataBufferSize;
	/// Keeps track of active amount of users for this texture.
	int users;

	/// Defines how sampling should be done for this texture. NEAREST will produce pixels while LINEAR smoothes. Default is LINEAR
	enum samplingModes {
		NEAREST,
		LINEAR,
	};
	int samplingMode;

	/// For using mipmaps. Default is true.
	bool mipmappingEnabled;
	/// Set this to true if you plan to bufferize this more than 1 time.
	bool dynamic;
	/// Set when rebufferization is queued. This flag is set for dynamic textures, for example for video produced by the MultimediaManager and its MultimediaStreams.
	bool queueRebufferization;
private:


	// Save via openCV
	bool SaveOpenCV(String toPath);
	/** An own PNG-saving function, since the library I'm using now is a tad slow (compared to OpenCV) while OpenCV demands 
		additional dependencies which may not be wanted!
	*/
	bool SavePNG(String toPath);

	/// Private ID as given on creation
	int id;
	static int IDenumerator;

	// Consider including other relevant info if using this class for image manipulation, mipmapping or whatever.
};

#endif
