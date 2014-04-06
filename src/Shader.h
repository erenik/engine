#ifndef SHADER_H
#define SHADER_H


#include "Uniform.h"

// Flags for stuff, like different error handling.
#define SHADER_REQUIRED 0x0000001

/** Defines a single shader program with it's separate shader program parts and IDs to all it's uniforms.
*/
class Shader 
{
public:
	Shader();
	~Shader();
	/// Deletes shader program and associated vertex/fragment-shaders
	void DeleteShader();
	char * source[2];
	char * name;
	int id;
	int flags;
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint shaderProgram;

	/// Flag for successful building of the shader
	bool built;
	/// Last time we updated information about the active lights in the shader
	long lightUpdate;

	/// Uniforms related to this specific shader
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

	/// Array of samplers
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
