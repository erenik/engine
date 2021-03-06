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
		/// Default 5.
		DIFFUSE = COLOR_BUFFER_1,
		NORMAL,
		POSITION,
		SPECULAR,
		EMISSIVE,
	};
};

namespace BufferStorageType 
{
	enum bufferTypes 
	{
		SINGLE_BYTE,
		/// Used for e.g. depth-buffers.
		DEPTH_16F, // 16-bit floating point. 
		DEPTH_24F, // 24-bit floating point depth.
		DEPTH_32F, // 32-bit floating point. Should work better for e.g. shadow mapping, but may cost some more memory.
		/// Vector-buffers.
		RGBA, // Also 8 bit per channel?
		RGBA_8, // 4-channels, 8 bit integers per channel.
		RGBA_16F, // 4-channels, 16-bit floating point.
		RGB_16F, // 3-channel, 16 bit floating point values per channel
		RGB_32F, // 3-channel, 32 bit floating point values per channel

	};
};

class RenderBuffer 
{
public:
	// See above.  NOTE: The RenderBuffer constructor is special in that it will initialize all GL-related objects within the constructor.
	RenderBuffer(String name, int type, int storageType, Vector2i size);
	/// Clean up! GL style.
	virtual ~RenderBuffer();
	void Free();

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
