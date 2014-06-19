// Emil Hedemalm
// 2013-07-03 Linuxification.

#include "ShaderManager.h"
#include "Globals.h"
#include "Graphics/GraphicsManager.h"
#include "GraphicsState.h"
#include <fstream>
#include <cstring>


/// Creates a shader using specified path/name .vert and .frag and returns a pointer to it
Shader * ShaderManager::CreateShader(const char * name, int i_flags)
{
	for (int i = 0; i < shaders.Size(); ++i){
		if (shaders[i]->name == name){
            std::cout<<"\nShader already exists! Returning it ^^";
            return shaders[i];
		}
	}

    std::cout<<"\nCreating shader "<<name;
	Shader * shade = new Shader();
	shade->name = new char [strlen(name)+1];
	shade->name[0] = NULL;
	strcat(shade->name, name);
	shade->flags = i_flags;

	const char shaderFolder[] = "shaders/";
	const char vertexShaderFileEnding[] = ".vert";
	const char fragmentShaderFileEnding[] = ".frag";
	shade->source[0] = new char [strlen(name)+strlen(shaderFolder)+strlen(vertexShaderFileEnding)+1];
	shade->source[0][0] = NULL;
	strcat(shade->source[0], shaderFolder);
	strcat(shade->source[0], name);
	strcat(shade->source[0], vertexShaderFileEnding);

	shade->source[1] = new char [strlen(name)+strlen(shaderFolder)+strlen(fragmentShaderFileEnding)+1];
	shade->source[1][0] = NULL;
	strcat(shade->source[1], shaderFolder);
	strcat(shade->source[1], name);
	strcat(shade->source[1], fragmentShaderFileEnding);
	shaders.Add(shade);
	return shade;
}

void ShaderManager::DeleteShaders(){
    while(shaders.Size()){
        Shader * shader = shaders[0];
		shader->DeleteShader();
		shaders.RemoveIndex(0);
		delete shader;
    }
	/// Unbind currently bound shader program so that it may be deallocated.
	glUseProgram(0);
}


/// Returns a shader program with the given name
Shader * ShaderManager::GetShaderProgram(String name){
	for (int i = 0; i < shaders.Size(); ++i){
	    Shader * shader = shaders[i];
		if (shader->name == 0)
			continue;
		if(name == shader->name){
			return shader;
		}
	}
	return NULL;
}
/// Recompiles target shader
bool ShaderManager::RecompileShader(Shader * shader){
	/// Initialize a SHADARRR
	const int LOG_MAX = 2048;
	char log[LOG_MAX];
	int i = 0;

    if (Graphics.GL_VERSION_MAJOR < 2){
        std::cout<<"\nOpenGL version below 2.0! Aborting recompilation procedure!";
        return false;
    }

#define ASSERT_IF_REQIRED {if (shader->flags & SHADER_REQUIRED) assert("Required shader failed to load! D:" && false);}

	std::cout<<"\nRecompiling shader: "<<shader->name;

	// Check if shaders and program have to be created first
	if (!shader->vertexShader)
		shader->vertexShader = glCreateShader(GL_VERTEX_SHADER);
	if (!shader->fragmentShader)
		shader->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (!shader->shaderProgram)
		shader->shaderProgram = glCreateProgram();

	// First read in our vertex shader!
	char * data;
	int size = 0, start = 0;
	String vertSource, fragSource;
	try {
		// Open file
		std::fstream file;
		file.open(shader->source[0], std::ios_base::in);
		if (!file.is_open()){
		    std::cout<<"\nUnable to open stream to file \""<<shader->source[0]<<"\"";
		    ASSERT_IF_REQIRED
		    return false;
		}
		// Extract length of file.
		start = (int)file.tellg();
		file.seekg(0, std::ios::end);
		size = ((int)file.tellg()) - start;
		file.seekg(0, std::ios::beg);
		// Allocate and read data.
		data = new char [size+5];
		memset(data, 0, size+5);
		file.read(data, size);
		// Close file again
		file.close();

		/// Add a null-sign ffs!
		data[size] = 0;

		if (file.fail()){

			if (file.fail())
				file.clear(std::ios_base::failbit);
			if (file.eof())
				file.clear(std::ios_base::eofbit);
			if (file.bad())
				file.clear(std::ios_base::badbit);
			if (file.good())

				// Reset bits
				int state = file.exceptions();
		}

		/* Associate the source code buffers with each handle */
		glShaderSource(shader->vertexShader, 1, (const GLchar **) &data, 0);
		checkGLError();
//		if (strcmp(shader->name, "Deferred") == 0)
//			std::cout<<"\nVertexShader Source: \n"<<data;
        // Copy data first!
		vertSource = String(data);
		// Deallocate data again
		delete[] data;
		data = NULL;
	} catch (...) {
		std::cout<<"\nFile I/O Error: Failed to read shader source from "<<shader->source[0];
		ASSERT_IF_REQIRED
		return false;
	}

	// Next read in our fragment shader!
	try {
		// Open file
		std::fstream file;
		file.open(shader->source[1], std::ios_base::in);
		if (file.fail()){
			int state = file.exceptions();
			throw 3;
		}
		// Extract length of file.
		start = (int)file.tellg();
		file.seekg(0, std::ios::end);
		size = ((int)file.tellg()) - start;
		file.seekg(0, std::ios::beg);
		// Allocate and read data.
		data = new char [size+5];
		memset(data, 0, size+5);
		file.read(data, size);
		// Close file again
		file.close();
		/// Add a null-sign ffs!
		data[size] = 0;


		/* Associate the source code buffers with each handle */
		glShaderSource(shader->fragmentShader, 1, (const GLchar **) &data, 0);
		checkGLError();
//		if (strcmp(shader->name, "Deferred") == 0)
//			std::cout<<"\nFragmentShader Source: \n"<<data;
        // Copy data first!
		fragSource = String(data);
		// Deallocate data again
		delete[] data;
		data = NULL;
	} catch (...) {
		std::cout<<"\nFile I/O Error: Failed to read shader source from "<<shader->source[1];
		ASSERT_IF_REQIRED
		return false;
	}

	/* Compile our shader objects */
	glCompileShader(shader->vertexShader);
	checkGLError()

	// Check if compilation failed.
	int status = NULL;
	glGetShaderiv(shader->vertexShader, GL_COMPILE_STATUS, &status);
	// If compilation failed, extract info log o-o
	int resultLength = 0;
	if (status == GL_FALSE){
		glGetShaderInfoLog(shader->vertexShader, LOG_MAX, &resultLength, log);
		if (resultLength > 0){
			// Just return if compilation fails for now. Enter a warning here or debug breakpoint if needed ^^
			String cError = "\nCompilation Error: ";
			cError += shader->source[0];
			cError += ": Failed compiling vertex shader: \n";
			cError += log;

			std::cout<<cError;
			glGetError();	// Release any eventual errors
			
			std::fstream dump;
			String dumpFileName = "ShaderCompilationError_VertexShader_";
			dumpFileName += shader->name;
			dump.open(dumpFileName.c_str(), std::ios_base::out);
			if (dump.is_open()){
				dump << cError;
			}
			dump.close();


			ASSERT_IF_REQIRED
			return false;
		}
	}

	glCompileShader(shader->fragmentShader);
	checkGLError()

	// Check if compilation failed.
	status = NULL;
	glGetShaderiv(shader->fragmentShader, GL_COMPILE_STATUS, &status);
	// If compilation failed, extract info log o-o
	resultLength = 0;
	if (status == GL_FALSE){
		glGetShaderInfoLog(shader->fragmentShader, LOG_MAX, &resultLength, log);

		if (resultLength > 0){

			// Just return if compilation fails for now. Enter a warning here or debug breakpoint if needed ^^
			String cError = "\nCompilation Error: ";
			cError += shader->source[1];
			cError += ": Failed compiling fragment shader: \n";
			cError += log;

			std::cout<<cError;
			glGetError();	// Release any eventual errors
			
			std::fstream dump;
			String dumpFileName = "ShaderCompilationError_FragmentShader_";
			dumpFileName += shader->name;
			dump.open(dumpFileName.c_str(), std::ios_base::out);
			if (dump.is_open()){
				dump << cError;
			}
			dump.close();

			// Just return if compilation fails for now. Enter a warning here or debug breakpoint if needed ^^
			std::cout<<cError;
			ASSERT_IF_REQIRED
			return false;
		}
	}

	/* Attach our shaders to our program */
	glAttachShader(shader->shaderProgram, shader->vertexShader);
	glAttachShader(shader->shaderProgram, shader->fragmentShader);
	GLuint error = glGetError();
	checkGLError();

	/* Bind attribute index 0 to vertex positions, attribute index 1 to incoming UV coordinates and index 2 to Normals. */
	glBindAttribLocation(shader->shaderProgram, 0, "in_Position");
	glBindAttribLocation(shader->shaderProgram, 1, "in_UV");
	glBindAttribLocation(shader->shaderProgram, 2, "in_Normal");
	glBindAttribLocation(shader->shaderProgram, 3, "in_Tangent");

	GLuint attrib = glGetAttribLocation(shader->shaderProgram, "in_Position");
	if (attrib == -1){
		std::cout<<"\nUnable to get attribute location for in_Position!";
	}
	attrib = glGetAttribLocation(shader->shaderProgram, "in_UV");
	if (attrib == -1){
		std::cout<<"\nUnable to get attribute location for in_UV!";
	}
	attrib = glGetAttribLocation(shader->shaderProgram, "in_Normal");
	if (attrib == -1){
		std::cout<<"\nUnable to get attribute location for in_Normal!";
	}
	attrib = glGetAttribLocation(shader->shaderProgram, "in_Tangent");
	if (attrib == -1){
		std::cout<<"\nUnable to get attribute location for in_Tangent!";
	}

	error = glGetError();
	checkGLError()

	/* Link our program, and set it as being actively used */
	glLinkProgram(shader->shaderProgram);
	checkGLError();
	// If compilation failed, extract info log o-o
	glGetProgramiv(shader->shaderProgram, GL_LINK_STATUS, &status);
	resultLength = 0;
	if (status == GL_FALSE){

		glGetProgramInfoLog(shader->shaderProgram, LOG_MAX, &resultLength, log);
		if (resultLength > 0){

			// Just return if compilation fails for now. Enter a warning here or debug breakpoint if needed ^^
			String cError = "\nLinker Error: ";
			cError += shader->source[1];
			cError += ": Failed linking shader: \n";
			cError += log;

			std::cout<<cError;
			glGetError();	// Release any eventual errors
			
			std::fstream dump;
			String dumpFileName = "ShaderLinkerError_FragmentShader_";
			dumpFileName += shader->name;
			dump.open(dumpFileName.c_str(), std::ios_base::out);
			if (dump.is_open()){
				dump << cError;
			}
			dump.close();

			// Just return if compilation fails for now. Enter a warning here or debug breakpoint if needed ^^
			std::cout<<cError;

			int error  = glGetError();	// Release any eventual errors
			glDetachShader(shader->shaderProgram, shader->fragmentShader);
			glDetachShader(shader->shaderProgram, shader->vertexShader);
			ASSERT_IF_REQIRED
			return false;
		}
	}

	checkGLError();
	/// TODO: Remove?
//	glUseProgram(shader->shaderProgram);
	checkGLError();


    GLint shaderProgram = shader->shaderProgram;
	//// Get uniform-location of the lights and set default values ^.^
	//for (int i = 0; i < 1; ++i){
	//	String	lstr = String("light");

	//	shader->uniformLight.activeBool = glGetUniformLocation(shaderProgram, (lstr+String("Active")).toAscii());
	//	glProgramUniform1i(shaderProgram, shader->uniformLight.activeBool, true);

	//	// Default ambient, diffuse and specular values
	shader->uniformLight.ambientVec4 = glGetUniformLocation(shaderProgram, "global_ambient");
	shader->uniformLight.diffuseVec4 = glGetUniformLocation(shaderProgram, "light_diffuse");
	shader->uniformLight.specularVec4 = glGetUniformLocation(shaderProgram, "light_specular");
	//
	//	// Type of light source.
	//	shader->uniformLight.typeInt = glGetUniformLocation(shaderProgram, (lstr+String("Type")).toAscii());
	//
	//	checkGLError();
	//	// Default direction or position coordinates
	//	shader->uniformLight.dirOrPosVec4 = glGetUniformLocation(shaderProgram, (lstr+String("DirOrPos")).toAscii());
	//
	//	// Default attenuation
	//	shader->uniformLight.attenuationVec3 = glGetUniformLocation(shaderProgram, (lstr+String("Attenuation")).toAscii());
	//}

	//GLuint uniform = glGetUniformLocation(shaderProgram, "lightDirOrPos");
	//if (uniform == -1){
	//	uniformLocatorResults += String("Unable to find location uniform \"light.dirOrPos\".\n");
	//}

	checkGLError();

	// Get Identifier pointers to the GL uniform projection/model matrices, once! o-o
	shader->uniformModelMatrix = glGetUniformLocation(shader->shaderProgram, "modelMatrix");
	shader->uniformViewMatrix = glGetUniformLocation(shader->shaderProgram, "viewMatrix");
	shader->uniformProjectionMatrix = glGetUniformLocation(shader->shaderProgram, "projectionMatrix");

	if (shader->uniformProjectionMatrix == -1){
		std::cout<<"\nUnable to find uniform for Projection Matrix";
	}
	if (shader->uniformViewMatrix == -1){
		std::cout<<"\nUnable to find uniform for View Matrix";
	}
	if (shader->uniformModelMatrix == -1){
		std::cout<<"\nUnable to find uniform for Model Matrix";
	}

	// Get camera uniforms ^^
	shader->uniformEyePosition = glGetUniformLocation(shader->shaderProgram, "eyePosition");
	if (shader->uniformEyePosition == -1){
		std::cout<<"\nUnable to find uniform for Eye Position";
	}

	checkGLError();
	// Set up projection matrix before sending it in
//	updateProjection();
//	state.projectionMatrixF = state.projectionMatrixD;

	// And send it in! o-o
	//glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, state.projectionMatrixF.getPointer());

	//// Get id for our eye position uniform
	//shader->uniformEyePosition = glGetUniformLocation(shaderProgram, "eyePosition");
	//// Get material properties now too.
	shader->uniformMaterial.ambientVec4 = glGetUniformLocation(shader->shaderProgram, "materialAmbient");
	shader->uniformMaterial.diffuseVec4 = glGetUniformLocation(shader->shaderProgram, "materialDiffuse");
	shader->uniformMaterial.specularVec4 = glGetUniformLocation(shader->shaderProgram, "materialSpecular");
	shader->uniformMaterial.shininessInt = glGetUniformLocation(shader->shaderProgram, "materialShininess");

	glActiveTexture(GL_TEXTURE0 + 0);
	shader->uniformBaseTexture = glGetUniformLocation(shader->shaderProgram, "baseImage");
	if (shader->uniformBaseTexture == -1){
		shader->uniformBaseTexture = glGetUniformLocation(shader->shaderProgram, "diffuseMap");
		if (shader->uniformBaseTexture == -1)
			std::cout<<"\nUniformBaseTexture \"baseImage/diffuseMap\" could not be located and set!";
	}

	glActiveTexture(GL_TEXTURE0 + 1);
	shader->uniformSpecularMap = glGetUniformLocation(shader->shaderProgram, "specularMap");
	if (shader->uniformSpecularMap == -1)
		std::cout<<"\nUniformSpecularMap \"specularMap\" could not be located and set!";

	glActiveTexture(GL_TEXTURE0 + 2);
	shader->uniformNormalMap = glGetUniformLocation(shader->shaderProgram, "normalMap");
	if (shader->uniformNormalMap == -1)
		std::cout<<"\nUniformNormalMap \"normalMap\" could not be located and set!";
	glActiveTexture(GL_TEXTURE0 + 0);

	/// Set the using uniforms to -1.
	shader->uniformUseDiffuseMap = glGetUniformLocation(shader->shaderProgram, "useDiffuseMap");
	shader->uniformUseSpecularMap = glGetUniformLocation(shader->shaderProgram, "useSpecularMap");
	shader->uniformUseNormalMap = glGetUniformLocation(shader->shaderProgram, "useNormalMap");

	shader->uniformPrimaryColorVec4 = glGetUniformLocation(shader->shaderProgram, "primaryColorVec4");
	if (shader->uniformPrimaryColorVec4 == -1)
		std::cout<<"\nUniform \"primaryColorVec4\" could not be located and set.";
	shader->uniformHighlightColorVec4 = glGetUniformLocation(shader->shaderProgram, "highlightColorVec4");

	// Foggy fog-some!
	shader->uniformFogBeginDistance = glGetUniformLocation(shader->shaderProgram, "fogBegin");
	shader->uniformFogEndDistance = glGetUniformLocation(shader->shaderProgram, "fogEnd");
	shader->uniformFogColor = glGetUniformLocation(shader->shaderProgram, "fogColor");


	std::cout<<"\n"<<shader->name<<" Shader Compilation Successful!\n";

	// Enable rendering now if it wasn't already
	shader->built = true;
	return true;
}

/// Attempts to recompile all shaders
void ShaderManager::RecompileAllShaders(){
    if (Graphics.GL_VERSION_MAJOR < 2){
        std::cout<<"\nOpenGL version below 2.0! Aborting recompilation procedure!";
        return;
    }
	for (int i = 0; i < shaders.Size(); ++i){
		RecompileShader(shaders[i]);
    }
}

/// Reload light data into all shaders that need the update.
bool informed = false;
void ShaderManager::ReloadLights(Lighting * newLighting){
	if (!informed){
		std::cout<<"\nWARNING: Implement ShaderManager::ReloadLights!";
		informed = true;
	}
	return;
	if (newLighting == NULL){
		std::cout<<"";
	}
}
