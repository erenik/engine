/// Emil Hedemalm
/// 2014-07-09
/// A buffer representing one layer of data for a specific viewport. 

#ifndef RENDER_BUFFER_H
#define RENDER_BUFFER_H

#include "MathLib.h"
#include "String/AEString.h"
#include "Graphics/OpenGL.h"
#include "Texture.h"

namespace BufferType 
{
	enum 
	{
		DEPTH_BUFFER,
		COLOR_BUFFER_1,
		COLOR_BUFFER_2,
		COLOR_BUFFER_3,
		COLOR_BUFFER_4,
		COLOR_BUFFER_5,
	};
};

namespace BufferStorageType 
{
	enum bufferTypes 
	{
		SINGLE_BYTE,
		DEPTH_16F, // 16-bit floating point. Used for e.g. depth-buffers.
		RGBA_8, // 4-channels, 8 bit integers per channel.
		RGB_16F, // 3-channel, 16 bit floating point values per channel
		RGB_32F, // 3-channel, 32 bit floating point values per channel

	};
};

class RenderBuffer 
{
public:
	// See above.  NOTE: The RenderBuffer constructor is special in that it will initialize all GL-related objects within the constructor.
	RenderBuffer(String name, int type, int storageType, Vector2i size);

	void CreateBuffer();
	/// Allocates a texture object to be used for storing/receiving and later on fetching values from this buffer.
	void CreateTexture();
	/// Binds this render buffer to the frame buffer which we are going to render to..!
	void BindTextureToFrameBuffer();

	// needed?
	String name;

	/// Type of buffer. See BufferType above.
	int type;
	int storageType;

	/// Internal formats.
	int glType, glStorageType;
	
	/// Size of the buffer in 2D-coordinates.
	Vector2i size;

	// The GL id for the render buffer.
	GLuint renderBuffer;
	// Internal storage.
	Texture * texture;

	int glid;
};

#endif
