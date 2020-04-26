/// Emil Hedemalm
/// 2015-02-11 
/// Separation between FrameBuffer and RenderBuffer classes/files.

#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include "RenderBuffer.h"

class GraphicsState;
class Shader;
class Viewport;

class FrameBuffer
{
public:
	// Default constructor?
	FrameBuffer(String name);
	FrameBuffer(Viewport * vp, Vector2i size);
	virtual ~FrameBuffer();
	const String & Name() const {return name;};
	void Free();
	void Nullify();

	void DumpTexturesToFile();
	/// Buffers will be written to. Number of written items will be returned.
	int GetDrawBuffers(GLenum * buffers);

	// Creates a set of default render-buffers.. might wanna re-visit and re-write this..
	void CreateRenderBuffers();
	/// Creates just 1 dedicated buffer to receive the depth-data. Preferably in 32-bit floating point to ensure better quality?
	bool CreateDepthBuffer(Vector2i size);
	/// Returns BufferStorageType::RGBA_16F or ::RGBA depending on settings.
	int ColorStorageType();
	/// Creates a default set of render-buffers for deferred rendering. Included: diffuse, normal, position, specular, emissive (all triple-vectors) + a depth buffer.   
	bool CreateDeferredBuffers(Vector2i size);
	/// Creates a default set of render-buffers for the output of rendering (deferred or otherwise), including color (diffuse) and depth.
	bool CreateDeferredOutputBuffers(Vector2i size);
	/// Creates 1 or more post-processing output buffers. More may be needed for ping-pong downscaling for e.g. Bloom
	bool CreatePostProcessOutputBuffers(Vector2i size);
	/// Yas. Only inherits main stats of the other buffer, keeps size.
	bool CreateBuffersLikeIn(FrameBuffer * otherBuffer);
	/// Binds it for use.
	bool Bind();
	/// Binds textures for sampling, e.g. using Deferred previously rendered-to buffer to do the Deferred-rendering part. Binds the textures straight into the current shader.
	void BindTexturesForSampling(Shader * inShader, GraphicsState& graphicsState);
	/// Sets the buffers to draw in.
	bool SetDrawBuffers();
	/// Yup.
	List<RenderBuffer*> renderBuffers;
	/// GLid for this thing.
	GLuint frameBufferObject;
	// Size.
	Vector2i size;
	/// If it is ready to be used.
	bool IsGood(){return good;};
	/// Default false.
	bool useFloatingPointStorage;
	bool skipDepthBuffers;
private:
	Viewport * viewport;
	// If it is ready to be used or not.
	bool good;
	String name;

	void CreateTexturesAndBind();
};

#endif
