/// Emil Hedemalm
/// 2014-07-10
/// OpenGL-based Shader class.

#include "Shader.h"

#include "Graphics/OpenGL.h"
#include "Globals.h"

#include <fstream>
#include <iostream>

#include "ShaderManager.h"

#include "Graphics/GraphicsManager.h"

//#include "Macros.h"

/// Ease-of-use function which grabs the active shader via the manager.
Shader * ActiveShader()
{
	return ShaderMan.ActiveShader();
}


Shader::Shader()
{
	/// GL ids
	shaderProgram = id = 0;
	vertexShader = fragmentShader = NULL;
	built = false;
	uniformModelMatrix = -1;
	uniformViewMatrix = -1;
	uniformProjectionMatrix = -1;

	uniformEyePosition = -1;

	for (int i = 0; i < 10; ++i)
		sampler[i] = -1;
	
	uniformBaseTexture = -1;
	uniformSpecularMap = -1;
	uniformNormalMap = -1;

	/// Set the using uniforms to -1.
	uniformUseDiffuseMap = uniformUseSpecularMap = uniformUseNormalMap = uniformBoneSkinningMatrixMap = -1;

	// Foggy fog-some!
	uniformFogBeginDistance = uniformFogEndDistance = uniformFogColor = -1;
}

Shader::~Shader()
{
	if (shaderProgram){
		std::cout<<"WARNING: "<<name<<" Shader program not deallocated!";
		assert(false && "shaderProgram not deallocated!");
	}
	if (vertexShader){
		std::cout<<"WARNING: "<<name<<" Vertex shader not deallocated!";
		assert(false && "vertexShader not deallocated!");
	}
	if (fragmentShader){
		std::cout<<"WARNING: "<<name<<" Fragment shader not deallocated!";
		assert(false && "fragmentShader not deallocated!");
	}

	shaderParts.ClearAndDelete();
	vertexShader = NULL;
	fragmentShader = NULL;
}

void Shader::DeleteShader()
{
	for (int i = 0; i < shaderParts.Size(); ++i)
	{
		ShaderPart * sp = shaderParts[i];
		sp->Delete();
	}
	if (shaderProgram){
		int deleteStatus = -1;
		glGetProgramiv(shaderProgram, GL_DELETE_STATUS, &deleteStatus);
		if (deleteStatus == GL_FALSE)
    		glDeleteProgram(shaderProgram);
		shaderProgram = NULL;
	}
	vertexShader = fragmentShader = 0;
}

// Attempts to compile this shader. Calls ExtractUniforms automatically upon completion.
bool Shader::Compile()
{
	int i = 0;

    if (GL_VERSION_MAJOR < 2){
        std::cout<<"\nOpenGL version below 2.0! Aborting recompilation procedure!";
        return false;
    }

	if (shaderParts.Size() == 0)
	{
		vertexShader = new ShaderPart(ShaderType::VERTEX_SHADER);
		shaderParts.Add(vertexShader);
		vertexShader->name = name;
		fragmentShader = new ShaderPart(ShaderType::FRAGMENT_SHADER);
		shaderParts.Add(fragmentShader);
		fragmentShader->name = name;
	}

	/// If we have made at least an attempt before.
	if (lastCompileAttempt.Type() != TimeType::UNDEFINED)
	{
		// Check the last edit time of the sources.
		Time mostRecentEdit = MostRecentEdit();
		// Skip compilation if we've already tried with the latest sources.
		if (mostRecentEdit < lastCompileAttempt)
		{
			std::cout<<"\nShader \'"<<name<<"\' up to date. Compile status: "<< (built ? "OK" : "Errors, see log files");
			/// Return last built state.
			return built;
		}
	}

	// Reset built flag.
	built = false;
	lastCompileAttempt = Time::Now();

	std::cout<<"\nRecompiling shader: "<<name;

	// Check if shaders and program have to be created first
	//if (!vertexShader)
	//	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//if (!fragmentShader)
	//	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (!shaderProgram)
		shaderProgram = glCreateProgram();

	// First read in our vertex shader!
	if (!vertexShader->Load(vertexSource))
		return false;
	if (!fragmentShader->Load(fragmentSource))
		return false;
	
	if (!vertexShader->Compile())
		return false;
	if (!fragmentShader->Compile())
		return false;
	
	/* Attach our shaders to our program */
	glAttachShader(shaderProgram, vertexShader->shaderKernelPart);
	glAttachShader(shaderProgram, fragmentShader->shaderKernelPart);
	GLuint error = glGetError();
	
	/* Bind attribute index 0 to vertex positions, attribute index 1 to incoming UV coordinates and index 2 to Normals. */
	glBindAttribLocation(shaderProgram, 0, "in_Position");
	glBindAttribLocation(shaderProgram, 1, "in_UV");
	glBindAttribLocation(shaderProgram, 2, "in_Normal");
	glBindAttribLocation(shaderProgram, 3, "in_Tangent");
	glBindAttribLocation(shaderProgram, 4, "in_BoneIndices");
	glBindAttribLocation(shaderProgram, 5, "in_BoneWeights");

	error = glGetError();

	/* Link our program, and set it as being actively used */
	glLinkProgram(shaderProgram);
	// If compilation failed, extract info log o-o
	GLint linkStatus;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
	int resultLength = 0;
	if (linkStatus == GL_FALSE){

		glGetProgramInfoLog(shaderProgram, LOG_MAX, &resultLength, shaderLog);
		if (resultLength > 0){

			// Just return if compilation fails for now. Enter a warning here or debug breakpoint if needed ^^
			String cError = "\nLinker Error: ";
			cError += name;
			cError += ": Failed linking shader: \n";
			cError += shaderLog;

			std::cout<<cError;
			glGetError();	// Release any eventual errors
			
			std::fstream dump;
			String dumpFileName = "ShaderLinkerError_FragmentShader_";
			dumpFileName += name;
			dump.open(dumpFileName.c_str(), std::ios_base::out);
			if (dump.is_open()){
				dump << cError;
			}
			dump.close();

			// Just return if compilation fails for now. Enter a warning here or debug breakpoint if needed ^^
			std::cout<<cError;

			int error  = glGetError();	// Release any eventual errors
			glDetachShader(shaderProgram, vertexShader->shaderKernelPart);
			glDetachShader(shaderProgram, fragmentShader->shaderKernelPart);
			return false;
		}
	}
	std::cout<<"\n"<<name<<" Shader Compilation Successful!\n";

	// Extract uniforms.
	ExtractUniforms();
	// .. and attributes!
	ExtractAttributes();

	// Enable rendering now if it wasn't already
	built = true;
	return true;
}


/// Extracts all uniforms required to set/adjust settings in the shader later on.
void Shader::ExtractUniforms()
{
	// Matrices

	// Get Identifier pointers to the GL uniform projection/model matrices, once! o-o
	uniformModelMatrix = glGetUniformLocation(shaderProgram, "modelMatrix");
	uniformViewMatrix = glGetUniformLocation(shaderProgram, "viewMatrix");
	uniformProjectionMatrix = glGetUniformLocation(shaderProgram, "projectionMatrix");
	uniformViewProjectionMatrix = glGetUniformLocation(shaderProgram, "viewProjectionMatrix");
	if (uniformProjectionMatrix == -1){
		std::cout<<"\nUnable to find uniform for Projection Matrix";
	}
	if (uniformViewMatrix == -1){
		std::cout<<"\nUnable to find uniform for View Matrix";
	}
	if (uniformModelMatrix == -1){
		std::cout<<"\nUnable to find uniform for Model Matrix";
	}

	uniformCameraRightWorldSpace = glGetUniformLocation(shaderProgram, "cameraRightWorldSpace");
	uniformCameraUpWorldSpace = glGetUniformLocation(shaderProgram, "cameraUpWorldSpace");
	uniformScale = glGetUniformLocation(shaderProgram, "scale");

	// Get camera uniforms ^^
	uniformEyePosition = glGetUniformLocation(shaderProgram, "eyePosition");
	if (uniformEyePosition == -1){
		std::cout<<"\nUnable to find uniform for Eye Position";
	}

	// Get textures
	// First diffuse
	glActiveTexture(GL_TEXTURE0 + 0);
	uniformBaseTexture = glGetUniformLocation(shaderProgram, "baseImage");
	if (uniformBaseTexture == -1)
	{
		uniformBaseTexture = glGetUniformLocation(shaderProgram, "diffuseMap");
		if (uniformBaseTexture == -1)
			std::cout<<"\nUniformBaseTexture \"baseImage/diffuseMap\" could not be located and set!";
	}
	// Specular map
	glActiveTexture(GL_TEXTURE0 + 1);
	uniformSpecularMap = glGetUniformLocation(shaderProgram, "specularMap");
	// Normal map
	glActiveTexture(GL_TEXTURE0 + 2);
	uniformNormalMap = glGetUniformLocation(shaderProgram, "normalMap");
	
	// Skinning matrix data map
	glActiveTexture(GL_TEXTURE0 + 3);
	uniformBoneSkinningMatrixMap = glGetUniformLocation(shaderProgram, "boneSkinningMatrixMap");

	// Shadow maps.
	glActiveTexture(GL_TEXTURE0 + 4);
	uniformShadowMap = glGetUniformLocation(shaderProgram, "shadowMap");
	uniformShadowMapMatrix = glGetUniformLocation(shaderProgram, "shadowMapMatrix");


	glActiveTexture(GL_TEXTURE0 + 0);

	/// Extract textures boolean uniforms.
	uniformUseDiffuseMap = glGetUniformLocation(shaderProgram, "useDiffuseMap");
	uniformUseSpecularMap = glGetUniformLocation(shaderProgram, "useSpecularMap");
	uniformUseNormalMap = glGetUniformLocation(shaderProgram, "useNormalMap");

	/// "Primary color"
	uniformPrimaryColorVec4 = glGetUniformLocation(shaderProgram, "primaryColorVec4");
	if (uniformPrimaryColorVec4 == -1)
		std::cout<<"\nUniform \"primaryColorVec4\" could not be located and set.";
	// Highlight color. Mainly used for UI.
	uniformHighlightColorVec4 = glGetUniformLocation(shaderProgram, "highlightColorVec4");

	// Foggy fog-some!
	uniformFogBeginDistance = glGetUniformLocation(shaderProgram, "fogBegin");
	uniformFogEndDistance = glGetUniformLocation(shaderProgram, "fogEnd");
	uniformFogColor = glGetUniformLocation(shaderProgram, "fogColor");


	//// Material properties 
	uniformMaterial.ambientVec4 = glGetUniformLocation(shaderProgram, "materialAmbient");
	uniformMaterial.diffuseVec4 = glGetUniformLocation(shaderProgram, "materialDiffuse");
	uniformMaterial.specularVec4 = glGetUniformLocation(shaderProgram, "materialSpecular");
	uniformMaterial.shininessInt = glGetUniformLocation(shaderProgram, "materialShininess");

	// Lighting uniforms.
	uniformLight.ambientVec4 = glGetUniformLocation(shaderProgram, "global_ambient");
	if (uniformLight.ambientVec4 == -1)
		uniformLight.ambientVec4 = glGetUniformLocation(shaderProgram, "light_ambient");
	uniformLight.diffuseVec4 = glGetUniformLocation(shaderProgram, "light_diffuse");
	uniformLight.specularVec4 = glGetUniformLocation(shaderProgram, "light_specular");

	// Light detail uniforms
	uniformLight.diffuseVec4 = glGetUniformLocation(shaderProgram, "light_diffuse");
	uniformLight.specularVec4 = glGetUniformLocation(shaderProgram, "light_specular");
	uniformLight.positionVec3 = glGetUniformLocation(shaderProgram, "light_position");
	uniformLight.attenuationVec3 = glGetUniformLocation(shaderProgram, "light_attenuation");
	uniformLight.castsShadowsBool = glGetUniformLocation(shaderProgram, "light_castsShadows");
	uniformLight.typeInt = glGetUniformLocation(shaderProgram, "light_type");
	uniformLight.spotDirectionVec3 = glGetUniformLocation(shaderProgram, "light_spotDirection");
	uniformLight.spotCutoffFloat = glGetUniformLocation(shaderProgram, "light_spotCutoff");
	uniformLight.spotExponentInt = glGetUniformLocation(shaderProgram, "light_spotExponent");
	
	// Particle uniforms.
	uniformParticleDecayAlphaWithLifeTime = glGetUniformLocation(shaderProgram, "particleDecayAlphaWithLifeTime");

	/// Extract attributes and uniforms as possible.
	uniforms.Clear();
	// get count
	int numUniforms;
	glGetProgramiv(shaderProgram, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &numUniforms);
	if (CheckGLError("Shader active uniforms") == GL_NO_ERROR)
	{
		for (int i = 0; i < numUniforms; ++i)
		{
			const int bufSize = 250;
			char nameBuf[bufSize];
			int size;
			int length;
			GLenum type;
			// for i in 0 to count:
			glGetActiveUniform (shaderProgram, i, bufSize, &length, &size, &type, nameBuf);
			GLSLIdentifier uniform;
			uniform.name = nameBuf;
			uniform.location = glGetUniformLocation(shaderProgram, nameBuf);
			uniform.type = type;
			uniform.size = size;
			uniforms.Add(uniform);
		}
	}
}

/// Extracts all attributes (input streams/buffers).
void Shader::ExtractAttributes()
{
	
	// Attributes for regular entity and UI rendering (older approaches, non-instanced, non-deferred, etc.)
	attributePosition = glGetAttribLocation(shaderProgram, "in_Position");
	attributeUV = glGetAttribLocation(shaderProgram, "in_UV");
	attributeNormal = glGetAttribLocation(shaderProgram, "in_Normal");
	attributeTangent = glGetAttribLocation(shaderProgram, "in_Tangent");

	// Attributes added with instanced particle rendering. 
	attributeVertexPosition = glGetAttribLocation(shaderProgram, "in_VertexPosition");
	attributeParticlePositionScale = glGetAttribLocation(shaderProgram, "in_ParticlePositionScale");
	attributeParticleLifeTimeDurationScale = glGetAttribLocation(shaderProgram, "in_ParticleLifeTimeDurationScale");
	attributeColor = glGetAttribLocation(shaderProgram, "in_Color");

	/// Attributes added with skeletal animation, vec4 both of them, thereby limiting number of bone-weights to 4 per vertex.
	attributeBoneIndices = glGetAttribLocation(shaderProgram, "in_BoneIndices");
	attributeBoneWeights = glGetAttribLocation(shaderProgram, "in_BoneWeights");

	/// Get them all enumerated nicely if lists are desired of all active attributes.
	// Just clear previous list first though.
	attributes.Clear();
	int numAttributes;
	glGetProgramiv(shaderProgram, GL_OBJECT_ACTIVE_ATTRIBUTES_ARB, &numAttributes);
	if (CheckGLError("Shader active attributes") == GL_NO_ERROR)
	{
		for (int i = 0; i < numAttributes; ++i)
		{
			const int bufSize = 250;
			char nameBuf[bufSize];
			int size;
			int length;
			GLenum type;
			glGetActiveAttrib(shaderProgram, i, bufSize, &length, &size, &type, nameBuf);
			GLSLIdentifier attrib;
			attrib.name = nameBuf;
			// Get the attribute location.
			attrib.location = glGetAttribLocation(shaderProgram, nameBuf);
			attrib.type = type;
			attrib.size = size;
			attributes.Add(attrib);
		}
	}
}

/// Prints the lists gathered in ExtractUniforms
void Shader::PrintAttributes()
{
	std::cout<<"\nTotal attributes: "<<attributes.Size();
	for (int i = 0; i < attributes.Size(); ++i)
	{
		GLSLIdentifier & attribute = attributes[i];
		std::cout<<"\nAttribute "<<i<<": "<<attribute.name<<", type: "<<attribute.type<<" size: "<<attribute.size;
	}
}
void Shader::PrintUniforms()
{
	std::cout<<"\nTotal uniforms: "<<uniforms.Size();
	for (int i = 0; i < uniforms.Size(); ++i)
	{
		GLSLIdentifier & uniform = uniforms[i];
		std::cout<<"\nAttribute "<<i<<": "<<uniform.name<<", type: "<<uniform.type<<" size: "<<uniform.size;
	}
}

/// Enables the respective vertex attribute pointers.
void Shader::OnMadeActive()
{
	CheckGLError("Before Shader::OnMadeActive");
	for (int i = 0; i < attributes.Size(); ++i)
	{
		GLSLIdentifier & attribute = attributes[i];
		glEnableVertexAttribArray(attribute.location);
	}
	CheckGLError("Shader::OnMadeActive -enabling vertex attrib arrays");
	SetTextureLocations();
	LoadLighting(graphicsState->lighting, this);
}
/// Disables the respective vertex attribute pointers.
void Shader::OnMadeInactive()
{
	for (int i = 0; i < attributes.Size(); ++i)
	{
		GLSLIdentifier & attribute = attributes[i];
		glDisableVertexAttribArray(attribute.location);
	}
}


/** Sets the texture indices to the default values, so that binding is done correctly afterwards. 
	The equivalent texture unit is glActiveTexture(GL_TEXTURE0 + value). Default values are as follows:
	0 - Diffuse/Default map
	1 - Specular map
	2 - Normal map
	3 - Bone skinning matrix map
*/
void Shader::SetTextureLocations()
{
	CheckGLError("Before Shader::SetTextureLocations");

	if (uniformBaseTexture != -1)
		glUniform1i(uniformBaseTexture, 0);
	if (uniformSpecularMap != -1)
		glUniform1i(uniformSpecularMap, 1);
	if (uniformNormalMap != -1)
		glUniform1i(uniformNormalMap, 2);
	if (uniformBoneSkinningMatrixMap != -1)
		glUniform1i(uniformBoneSkinningMatrixMap, 3); // Sets sampler to use texture #3 for skinning maps	

	// Location 4 and onwards for the shadow maps?
	if (uniformShadowMap != -1)
		glUniform1i(uniformShadowMap, 4);

	// Un-bind all previous texture!
	for (int i = 0; i < 5; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glActiveTexture(GL_TEXTURE0);

	CheckGLError("Shader::SetTextureLocations");
}


void Shader::SetupFog(GraphicsState & graphicsState)
{
	if (uniformFogBeginDistance != -1)
	{
		glUniform1f(uniformFogBeginDistance, graphicsState.fogBegin);
		glUniform1f(uniformFogEndDistance, graphicsState.fogEnd);
		glUniform3f(uniformFogColor, graphicsState.clearColor[0], graphicsState.clearColor[1], graphicsState.clearColor[2]);
	}
}

/// Returns the most recent edit time of the shader parts this program constitutes of.
Time Shader::MostRecentEdit()
{
	Time mostRecentEdit;
	for (int i = 0; i < shaderParts.Size(); ++i)
	{
		ShaderPart * part = shaderParts[i];
		Time lastModified = part->source.LastModified();
		if (lastModified > mostRecentEdit)
			mostRecentEdit = lastModified;
	}
	return mostRecentEdit;
}

