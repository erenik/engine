/// Emil Hedemalm
/// 2014-07-10
/// OpenGL-based Shader class.

#ifndef SHADER_H
#define SHADER_H

#include "Uniform.h"
#include "ShaderPart.h"

struct GLSLIdentifier
{
	String name;
	/// Contains the GLSL specific ID or location of this identifier. This is then used by various functions for interacting with it.
	int location;
	/** The type argument will return a
		pointer to the attribute variable's data type. The symbolic
		constants GL_FLOAT,
		GL_FLOAT_VEC2,
		GL_FLOAT_VEC3,
		GL_FLOAT_VEC4,
		GL_FLOAT_MAT2,
		GL_FLOAT_MAT3, or
		GL_FLOAT_MAT4 may be returned. The
		size argument will return the size of the
		attribute, in units of the type returned in
		type.
	*/
	int type;
	// Size in bytes.
	int size;
};

// Flags for stuff, like different error handling.
#define SHADER_REQUIRED 0x0000001

class Shader;

/// Ease-of-use function which grabs the active shader via the manager.
Shader * ActiveShader();

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
	/// Extracts all attributes (input streams/buffers).
	void ExtractAttributes();
	/// Extracts all uniforms required to set/adjust settings in the shader later on.
	void ExtractUniforms();
	
	/// Prints the lists gathered in ExtractUniforms
	void PrintAttributes();
	void PrintUniforms();

	/// Enables the respective vertex attribute pointers.
	void OnMadeActive();
	/// Disables the respective vertex attribute pointers.
	void OnMadeInactive();

	/** Sets the texture indices to the default values, so that binding is done correctly afterwards. 
		The equivalent texture unit is glActiveTexture(GL_TEXTURE0 + value). Default values are as follows:
		0 - Diffuse/Default map
		1 - Specular map
		2 - Normal map
		3 - Bone skinning matrix map
	*/
	void SetTextureLocations();

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

	/** Attribues (streamed/buffered data) specific to this shader.
		Attributes not in use will have the value -1 (or UINT_MAX?)
		Their GLSL equivalent are usually in_NameMinusStartingAttribute, e.g. in_VertexPosition for attributeVertexPosition
	*/
	/// Attributes for regular entity and UI rendering (older approaches, non-instanced, non-deferred, etc.)
	GLuint attributePosition, attributeUV, attributeNormal, attributeTangent;

	/// Attributes added with instanced particle rendering. 
	GLuint attributeVertexPosition, // Vertex XYZ for the model which is instanced.
		attributeParticlePositionScale, /// World position XYZ and scale stored in the W component 
		attributeColor, /// Color XYZW for tweaking each individual instance/particle.
		attributeParticleLifeTimeDurationScale; // Life time (total), duration (current), as well as scale in X and Y.
 
	/// Attributes added with skeletal animation, vec4 both of them, thereby limiting number of bone-weights to 4 per vertex.
	GLuint attributeBoneIndices;
	GLuint attributeBoneWeights;


	/// Uniforms related to this specific shader
	/// ========================================
	/// GL uniform model matrix identifier
	GLuint uniformModelMatrix;
	/// GL uniform view matrix identifier
	GLuint uniformViewMatrix;
	/// GL uniform projection matrix identifier
	GLuint uniformProjectionMatrix;

	GLuint uniformCameraRightWorldSpace;
	GLuint uniformCameraUpWorldSpace;

	/// Used to apply a scale to all, in addition to per-particle scale/stuffs.
	GLuint uniformScale;

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

	/// Particle uniforms.
	GLuint uniformParticleDecayAlphaWithLifeTime;

	/// Array of (texture) samplers
	GLuint sampler[10];		// Max 10 simultaneous samplers

	/// Texture uniforms.
	GLuint uniformBaseTexture,
		uniformSpecularMap,
		uniformNormalMap;
	/// Skeletal animation texture storages
	GLuint uniformBoneSkinningMatrixMap;

	GLuint uniformUseDiffuseMap, 
		uniformUseSpecularMap, 
		uniformUseNormalMap;


	// Foggy fog-some!
	GLuint uniformFogBeginDistance,
		uniformFogEndDistance,
		uniformFogColor;

	static const int LOG_MAX = 2048;
	char shaderLog[LOG_MAX];


	/// For automatically parsed attributes and uniforms.
	List<GLSLIdentifier> attributes;
	List<GLSLIdentifier> uniforms;

	/// Returns the most recent edit time of the shader parts this program constitutes of.
	Time MostRecentEdit();

	Time lastCompileAttempt;

protected:
};


#endif
