/// Emil Hedemalm
/// 2014-09-16
/// Specifies an individual shader part (a.k.a. kernel)

#ifndef SHADER_PART_H
#define SHADER_PART_H

#include "String/AEString.h"
#include "Graphics/OpenGL.h"
#include "File/File.h"

namespace ShaderType {
	enum {
		VERTEX_SHADER,
		FRAGMENT_SHADER,
		GEOMETRY_SHADER,
	};
};

class Shader;

/// E.g. Vertex shader, Fragment shader, etc. All of these are then embedded into the main Shader object.
class ShaderPart 
{
public:
	/// 
	ShaderPart(int type, Shader * shader);
	/// Deletes the GL-related items.
	void Delete();
	String name;
	// Loads from source. Must be run from a render-thread, as it loads the source straight into the shader-kernel/-part.
	bool Load(String fromSource);
	void CreateShaderKernelIfNeeded();
	// Copmpiles it, returning the result.
	bool Compile();
	/// See ShaderType above.
	int type;
	

	File source;

	String sourceCode;
	/// The "shader" ID as it is known in GL that will be a reference ID for handling this specific shader part.
	GLuint shaderKernelPart;

private:
	static const int LOG_MAX = 2048;
	char shaderLog[LOG_MAX];
	Shader * shader;
};

#endif
