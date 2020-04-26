/// Emil Hedemalm
/// 2014-11-13 (although older)
/// Manager for rendering shaders (primarily OpenGL)

#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include "Graphics/OpenGL.h"

#include "Shader.h"
#include "Lighting.h"
#include "List/List.h"
class Lighting;

#define MAX_SHADERS 32

#define ShadeMan ShaderMan
#define ShaderMan (*ShaderManager::Instance())

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
	void DeleteShaders(GraphicsState & graphicsState);

	
	/// Sets selected shader to be active.  Returns the shader that was set.
	Shader * SetActiveShader(Shader * shader, GraphicsState & graphicsState);
	/** Sets selected shader to be active. Prints out error information and does not set to new shader if it fails.
		Returns the active shading program or NULL if it fails.
		WARNING: Should only be called from a render-thread, yaow.
	*/
	Shader * SetActiveShader(String shaderName, GraphicsState& graphicsState);
	/// Returns the active shader, or NULL if the default shading program is in use.
	Shader * ActiveShader();

	/// Returns a shader with the given name
	Shader * GetShader(String name);
	/// Recompiles target shader
	bool RecompileShader(Shader * shader);
	/// Attempts to recompile all shaders
	void RecompileAllShaders();
	/// Reload light data into all shaders that need the update.
	void ReloadLights(Lighting * newLighting);
private:
	/// o.o 
	Shader * activeShader;

	List<Shader*> shaders;
};


#endif
