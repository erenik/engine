/// Emil Hedemalm
/// 2015-02-11 
/// Separation between FrameBuffer and RenderBuffer classes/files.

#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include "RenderBuffer.h"

class Shader;
class Viewport;

class FrameBuffer
{
public:
	// Default constructor?
	FrameBuffer(String name);
	FrameBuffer(Viewport * vp, Vector2i size);
	virtual ~FrameBuffer();
	void Nullify();

	void DumpTexturesToFile();
	/// Buffers will be written to. Number of written items will be returned.
	int GetDrawBuffers(GLenum * buffers);

	// Creates a set of default render-buffers.. might wanna re-visit and re-write this..
	void CreateRenderBuffers();
	/// Creates just 1 dedicated buffer to receive the depth-data. Preferably in 32-bit floating point to ensure better quality?
	bool CreateDepthBuffer(Vector2i size);
	/// Creates a default set of render-buffers for deferred rendering. Included: diffuse, normal, position, specular, emissive (all triple-vectors) + a depth buffer.   
	bool CreateDeferredBuffers(Vector2i size);
	/// Binds it for use.
	bool Bind();
	/// Binds textures for sampling, e.g. using Deferred previously rendered-to buffer to do the Deferred-rendering part. Binds the textures straight into the current shader.
	void BindTexturesForSampling(Shader * inShader);
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
private:
	Viewport * viewport;
	// If it is ready to be used or not.
	bool good;
	String name;
};

#endif
