
#ifndef LIGHTING_H
#define LIGHTING_H

class GraphicsState;

#include "Light.h"
class Light;
class Shader;
class Message;
class AppWindow;

#define MAX_LIGHTS 128

/** A structure for keeping track of currently active lights in a scene
	It contains mostly pointers since it is an encapsulating object for passing data between functions.
*/
class 
#ifdef USE_SSE
	alignas(16)
#endif
	Lighting
{
	/// Loads selected lighting into the active shader program
	friend void LoadLighting(Lighting * lighting, Shader * shader);
	friend class GMAddLight;
public:
	String name;

	/// Default constructor, sets all light pointers and other variables to 0/NULL
	Lighting();
	~Lighting();

	/// Returns true if the message had any meaning, adjusting values within the lighting.
	bool ProcessMessage(Message * message);
	/// Opens up an editor-AppWindow for this lighting, assuming the existance of LightingMenu and LightEditor GUI files.
	AppWindow * OpenEditorWindow();
	/// Updates UI for all lights in this lighting. If AppWindow is not specified, the default AppWindow will be requested.
	void UpdateLightList(AppWindow * inWindow = NULL);

	/// Creates a new light to this setup.
	Light * NewLight(String name);
	/// Adds light to the lighting, return NULL upon falure. 
	Light * Add(Light * light);
	/// Removes target light. Returns false on failure.
	bool Remove(Light * light);
	/// Creates a default setup of 3-4 lights for testing purposes.
	void CreateDefaultSetup();
	/// Returns all lights in their current state.
	List<Light*> GetLights() const;
	int NumLights() const;
	/// Fills the whole light-array, attempting to  test the limits of the hardware with as many lights as possible.
	void Overload();
	/// Sets ambient using integral values (divided by 255.0 to get float value)
	void SetAmbient(int r, int g, int b, int a = 255);
	/// Sets ambient using floats.
	void SetAmbient(float r, float g, float b, float a = 1.0f);
	/// Sets ambient using doubles.
	void SetAmbient(double r, double g, double b, double a = 1.0);
	/// Sets ambient values. Alpha defaults to 0.
	void SetAmbient(const Vector3f & values);
	/// Returns the current values for the global ambient light.
	Vector4f GetAmbient() const { return Vector4f(global_ambient); };
	/// Creates a new light source, returning it. NULL on falure.
	Light * CreateLight();


	/// Selects and makes active.
	Light * SelectLightByIndex(int index);
	/// Selects and makes active target light. May return NULL.
	Light * SelectLightByName(String byName);
	
	/// Returns a pointer to the active light. USE WITH CAUTION.
	Light * GetLight() { return activeLight; };
	/// Returns a pointer to selected light. USE WITH CAUTION.
	Light * GetLight(int index);
	/// Deletes active light source or target light if argument is provided. Returns false if no light is selected or hte light did not belong to this lighting.
	bool DeleteLight(Light * light = NULL);
	/// Deletes all light sources contained within.
	int DeleteAllLights();

	/// Loads from target file, calling ReadFrom once a valid stream has been opened. Returns false if it failed to opened the stream.
	bool LoadFrom(String fileName);
	/// Writes to file stream.
	void WriteTo(std::fstream & file);
	/// Reads from file stream.
	void ReadFrom(std::fstream & file);

	/// wooo
	Vector3f skyColor;
	/// ID of last frame that we prepared for loading stats into GL etc.
	int lastPreparationFrame;

	/// Fills the big arrays with data from the individual lights.
	void PrepareForLoading(GraphicsState* graphicsState);

	/// Big arrays for loading all lighting data in one go.
	int activeLights;
	float ambient[4];
	float diffuse[MAX_LIGHTS * 4];
	float specular[MAX_LIGHTS * 4];
	float position[MAX_LIGHTS * 3];
	float attenuation[MAX_LIGHTS * 3];
	int castsShadows[MAX_LIGHTS];
	int type[MAX_LIGHTS];
	float spotDirection[MAX_LIGHTS * 3];
	float spotCutoff[MAX_LIGHTS];
	int spotExponent[MAX_LIGHTS];

	/// Loads selected lighting into the active shader program
	void LoadIntoShader(GraphicsState* graphicsState, Shader * shader);

private:
	bool SaveLighting(String toFileName);
	bool LoadLighting(String fromFileName);

	/// Pointer to array of 4 floats (rgba)
	Vector4f global_ambient;
	/// Array of [MAX_LIGHTS] lights.
	List<Light*> lights;
	/// Last time we changed any of the internal parameters for this current lighting, in ms (Timer::GetCurrentTimeMs())
	AETime lastUpdate;
	/// Currently active light for editing purposes ^^
	int activeLightIndex;

	/// Currently active light pointer
#define ACTIVE_LIGHT light[activeLightIndex]
	Light * activeLight;
	/// Number of lights we've created
	int lightCounter;


	/// o.o
	static AppWindow * lightingEditor;
};

#endif
