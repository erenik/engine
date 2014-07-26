/// Emil Hedemalm
/// 2014-07-10
/// OpenGL-based Shader class.

#ifndef SHADER_H
#define SHADER_H

#include "Uniform.h"
#include "String/AEString.h"

// Flags for stuff, like different error handling.
#define SHADER_REQUIRED 0x0000001

namespace ShaderType {
	enum {
		VERTEX_SHADER,
		FRAGMENT_SHADER,
		GEOMETRY_SHADER,
	};
};

/// E.g. Vertex shader, Fragment shader, etc. All of these are then embedded into the main Shader object.
class ShaderPart 
{
public:
	/// 
	ShaderPart(int type);
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
	String source;
	/// The "shader" ID as it is known in GL that will be a reference ID for handling this specific shader part.
	GLuint shaderKernelPart;
};

/** Defines a single shader program with it's separate shader program parts and IDs to all it's uniforms.
*/
class Shader 
{
public:
	Shader();
	~Shader();
	
	/// Deletes shader program and associated vertex/fragment-shaders
	void DeleteShader();

	// Attempts to compile this shader. Calls ExtractUniforms automatically upon completion.
	bool Compile();
	/// Extracts all uniforms required to set/adjust settings in the shader later on.
	void ExtractUniforms();

	String name;
	String vertexSource;
	String fragmentSource;

	/// will contain the separate shader-parts.
	List<ShaderPart*> shaderParts; 
	ShaderPart * vertexShader, * fragmentShader;

	int id;
	int flags;
//	GLuint vertexShader;
//	GLuint fragmentShader;
	GLuint shaderProgram;

	/// Flag for successful building of the shader
	bool built;
	/// Last time we updated information about the active lights in the shader
	long lightUpdate;


	/// Uniforms related to this specific shader
	/// ========================================
	/// GL uniform model matrix identifier
	GLuint uniformModelMatrix;
	/// GL uniform view matrix identifier
	GLuint uniformViewMatrix;
	/// GL uniform projection matrix identifier
	GLuint uniformProjectionMatrix;

	/// GL uniform Eye position
	GLuint uniformEyePosition;

	/// Primary color (for shading/wireframes/special effects).
	GLuint uniformPrimaryColorVec4;
	/// Linearly added color, primarily used for UI highlighting.
	GLuint uniformHighlightColorVec4;

	/// GL uniform material ID
	UniformMaterial uniformMaterial;

	/// GL uniform light identifiers
	UniformLight uniformLight;

	/// Array of (texture) samplers
	GLuint sampler[10];		// Max 10 simultaneous samplers

	/// Texture binding uniform ID
	GLuint uniformBaseTexture;
	GLuint uniformSpecularMap;
	GLuint uniformNormalMap;

	GLuint uniformUseDiffuseMap, 
		uniformUseSpecularMap, 
		uniformUseNormalMap;

	// Foggy fog-some!
	GLuint uniformFogBeginDistance,
		uniformFogEndDistance,
		uniformFogColor;

};


#endif
