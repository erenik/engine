

#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <GL/glew.h>
#include "Shader.h"
#include "Lighting.h"
#include "List/List.h"
class Lighting;

#define MAX_SHADERS 32

class ShaderManager{
public:
	/// Creates a shader using specified path/name .vert and .frag and returns a pointer to it
	Shader * CreateShader(const char * name, int flags = 0);
	/// Deletes all shaders
	void DeleteShaders();
	/// Returns a shader with the given name
	Shader * GetShaderProgram(String name);
	/// Recompiles target shader
	bool RecompileShader(Shader * shader);
	/// Attempts to recompile all shaders
	void RecompileAllShaders();
	/// Reload light data into all shaders that need the update.
	void ReloadLights(Lighting * newLighting);
private:
	List<Shader*> shaders;
};


#endif
