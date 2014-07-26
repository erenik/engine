

#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <GL/glew.h>
#include "Shader.h"
#include "Lighting.h"
#include "List/List.h"
class Lighting;

#define MAX_SHADERS 32

#define ShadeMan (*ShaderManager::Instance())

class ShaderManager
{
	ShaderManager();
	~ShaderManager();
	static ShaderManager * shaderManager;
public:
	static void Allocate();
	static void Deallocate();
	static ShaderManager * Instance();

	// Old method. Should maybe be replaced.
	void CreateDefaultShaders();
	/// Creates a shader using specified path/name .vert and .frag and returns a pointer to it
	Shader * CreateShader(const char * name, int flags = 0);
	/// Deletes all shaders
	void DeleteShaders();

	
	/// Sets selected shader to be active.  Returns the shader that was set.
	Shader * SetActiveShader(Shader * shader);
	/** Sets selected shader to be active. Prints out error information and does not set to new shader if it fails.
		Returns the active shading program or NULL if it fails.
		WARNING: Should only be called from a render-thread, yaow.
	*/
	Shader * SetActiveShader(String shaderName);

	/// Returns a shader with the given name
	Shader * GetShader(String name);
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
