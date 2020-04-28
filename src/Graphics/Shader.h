/// Emil Hedemalm
/// 2014-07-10
/// OpenGL-based Shader class.

#ifndef SHADER_H
#define SHADER_H

#include "Uniform.h"
#include "ShaderPart.h"

class Matrix4f;

struct GLSLIdentifier
{
	GLSLIdentifier();
	String name;
	/// Used for attributes o.o
	bool defaultEnable; 
	bool isAttribute;
	bool isUniform;
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
class GraphicsState;

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
	void OnMadeActive(GraphicsState * graphicsState);
	/// Disables the respective vertex attribute pointers.
	void OnMadeInactive();
	
	/// Setters, added for laziness, but may be useful if going non-GL later, I guess?
	void SetProjectionMatrix(const Matrix4f & mat);
	/// Setters, added for laziness, but may be useful if going non-GL later, I guess?
	void SetViewMatrix(const Matrix4f & mat);
	void SetModelMatrix(const Matrix4f & mat);

	/// See Render/RenderBuffer.h for types.
	int GetUniformTextureByBufferType(int type);

	/** Sets the texture indices to the default values, so that binding is done correctly afterwards. 
		The equivalent texture unit is glActiveTexture(GL_TEXTURE0 + value). Default values are as follows:
		0 - Diffuse/Default map
		1 - Specular map
		2 - Normal map
		3 - Bone skinning matrix map
	*/
	void SetTextureLocations();
	/// Sets up fog uniforms as possible.
	void SetupFog(GraphicsState & graphicsState);

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
	AETime lastLightUpdate;

	/** Attribues (streamed/buffered data) specific to this shader.
		Attributes not in use will have the value -1 (or UINT_MAX?)
		Their GLSL equivalent are usually in_NameMinusStartingAttribute, e.g. in_VertexPosition for attributeVertexPosition
	*/
	/** Attributes for regular entity and UI rendering (older approaches, non-instanced, non-deferred, etc.)
		Respective in-shader names are in_Position, in_UV, in_Normal, etc.
	*/
	GLuint attributePosition, // the same as attributeVertexPosition, not positions of instances.
		attributeUV, attributeNormal, attributeTangent, attributeBiTangent;

	/// Attributes added with instanced particle rendering. 
	GLuint attributeParticlePositionScale, /// World position XYZ and scale stored in the W component 
		attributeColor, /// Color XYZW for tweaking each individual instance/particle.
		attributeParticleLifeTimeDurationScale; // Life time (total), duration (current), as well as scale in X and Y.
 
	/// Attributes added with skeletal animation, vec4 both of them, thereby limiting number of bone-weights to 4 per vertex.
	GLuint attributeBoneIndices;
	GLuint attributeBoneWeights;

	/// Other instances attributes for when instancing entities that need to be rendered.
	GLuint attributeInstanceModelMatrix,
		attributeInstanceNormalMatrix;

	/// All attributes pertaining to instancing. These are not enabled by default when activating the shader.
	List<GLuint> instanceAttributes;

	/// Used to booleanly switch between instancing and individual rendering in the shader.
	GLuint uniformInstancingEnabled;

	/// Uniforms related to this specific shader
	/// ========================================
	/// GL uniform model matrix identifier
	GLuint uniformModelMatrix;
	GLuint uniformViewMatrix;
	GLuint uniformProjectionMatrix;
	GLuint uniformViewProjectionMatrix; // Will replace View and Projection matrices?
	GLuint uniformNormalMatrix;

	GLuint uniformCameraRightWorldSpace;
	GLuint uniformCameraUpWorldSpace;

	/// Used for Font-shaders.
	GLuint uniformCharacter; // Integer of character, 0 to 255 in a 16x16 grid.
	GLuint uniformPivot; // XY-coordinate.. relative to UI or something.
	/** See shaders/Font.frag for details concerning implementation. Also TextFont and Text classes are relevant.
		0 - Default. Pass through.
		1 - Simple White font - apply primaryColorVec4 multiplicatively
		2 - Replacer. Replaces a set amount of colors in the font for other designated colors (primarily primaryColorVec4 and se)
	*/
	GLuint uniformColorEquation;


	/// Used to apply a scale to all, in addition to per-particle scale/stuffs.
	GLuint uniformScale;

	/// GL uniform Eye position
	GLuint uniformEyePosition;

	/// Primary color (for shading/wireframes/special effects).
	GLuint uniformPrimaryColorVec4;
	/// Linearly added color, primarily used for UI highlighting.
	GLuint uniformHighlightColorVec4;
	// Boolean, for highlighting text.
	GLuint uniformHoveredOver; 

	/// GL uniform material ID
	UniformMaterial uniformMaterial;

	/// o.o
	GLuint uniformSunPosition; // Vec3
	GLuint uniformSunColor; // Vec4
	GLuint uniformSkyColor; // Vec3
	/// GL uniform light identifiers
	UniformLight uniformLight;

	/// Particle uniforms.
	GLuint uniformParticleDecayAlphaWithLifeTime;

	/// Array of (texture) samplers
	GLuint sampler[10];		// Max 10 simultaneous samplers

	/** Texture uniforms, per-entity.
		Default IDs/#s of them:
		0 - Diffuse
		1 - Specular
		2 - Normal
		3 - Emissive
		4 - Bone/Skinning
	*/
#define uniformDiffuseMap uniformBaseTexture
	GLuint uniformBaseTexture,
		uniformSpecularMap,
		uniformNormalMap,
		uniformEmissiveMap;
	GLuint uniformBoneSkinningMatrixMap;	/// Skeletal animation texture storages

	/// Maps for deferred et al.
	GLuint uniformPositionMap, uniformDepthMap;

	// Shadow maps.
	GLuint uniformShadowMap; // Default location: glActiveTexture(GL_TEXTURE0 + 4);
	GLuint uniformShadowMapMatrix;

	GLuint uniformExposure; /// Used for ToneMapping in HDR rendering pipelines.

	/// Provides the locations for the above, if they should deviate from the defaults.
	int diffuseMapIndex;
	int specularMapIndex;
	int normalMapIndex;
	int emissiveMapIndex;
	int boneSkinningMatrixMapIndex;
	int shadowMapIndex;
	int positionMapIndex, depthMapIndex;
	
	// Texture factors
	GLuint uniformEmissiveMapFactor;


	GLuint uniformUseDiffuseMap, 
		uniformUseSpecularMap, 
		uniformUseNormalMap;


	// Foggy fog-some!
	GLuint uniformFogBeginDistance,
		uniformFogEndDistance,
		uniformFogColor;

	static const int LOG_MAX = 2048;
	char shaderLog[LOG_MAX];


	/// Default -1. Should be parsed from the file. e.g: #define MAX_LIGHTS 32
	int maxLights;

	/// For automatically parsed attributes and uniforms.
	List<GLSLIdentifier> attributes;
	List<GLSLIdentifier> uniforms;

	/// Returns the most recent edit time of the shader parts this program constitutes of.
	Time MostRecentEdit();

	Time lastCompileAttempt;

protected:
	List<GLuint> defaultDisabledAttributes;
};


#endif
