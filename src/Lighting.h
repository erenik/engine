
#ifndef LIGHTING_H
#define LIGHTING_H

class GraphicsState;

#include "Light.h"
class Light;
class Shader;
class Message;
class Window;

#define MAX_LIGHTS 128

/** A structure for keeping track of currently active lights in a scene
	It contains mostly pointers since it is an encapsulating object for passing data between functions.
*/
class Lighting {
	/// Loads selected lighting into the active shader program
	friend void LoadLighting(Lighting * lighting, Shader * shader);
#define GetTime clock
public:
	String name;

	/// Default constructor, sets all light pointers and other variables to 0/NULL
	Lighting();
	~Lighting();
	/// Copy constructor ^^
	Lighting(const Lighting& lighting);
	/// Assignment operator...
	const Lighting * operator = (const Lighting & otherLighting);

	/// Returns true if the message had any meaning, adjusting values within the lighting.
	bool ProcessMessage(Message * message);
	/// Opens up an editor-window for this lighting, assuming the existance of LightingMenu and LightEditor GUI files.
	Window * OpenEditorWindow();
	/// Updates UI for all lights in this lighting. If window is not specified, the default window will be requested.
	void UpdateLightList(Window * inWindow = NULL);

	/// Creates a new light to this setup.
	Light * NewLight(String name);
	/// DEPRECATE: bad arguments... Adds light to the lighting, return NULL upon falure. Note that the light is copied in this case! TODO: Remove this function.
	Light * Add(Light * light);
	/// Removes target light. Returns false on failure.
	bool Remove(Light * light);
	/// Creates a default setup of 3-4 lights for testing purposes.
	void CreateDefaultSetup();
	/// Returns all lights in their current state.
	List<Light*> GetLights() const;
	/// Fills the whole light-array, attempting to  test the limits of the hardware with as many lights as possible.
	void Overload();
	/// Sets ambient using integral values (divided by 255.0 to get float value)
	void SetAmbient(int r, int g, int b, int a = 255);
	/// Sets ambient using floats.
	void SetAmbient(float r, float g, float b, float a = 1.0f);
	/// Sets ambient using doubles.
	void SetAmbient(double r, double g, double b, double a = 1.0);
	/// Sets ambient values. Alpha defaults to 0.
	void SetAmbient(Vector3f values);
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
private:
	/// Used for all copy-constructors.
	void Copy(const Lighting * fromThisLighting);

	/// Pointer to array of 4 floats (rgba)
	Vector4f global_ambient;
	/// Array of [MAX_LIGHTS] lights.
	List<Light*> lights;
	/// Last time we changed any of the internal parameters for this current lighting, in ms (Timer::GetCurrentTimeMs())
	long lastUpdate;
	/// Currently active light for editing purposes ^^
	int activeLightIndex;

	/// Currently active light pointer
#define ACTIVE_LIGHT light[activeLightIndex]
	Light * activeLight;
	/// Number of lights we've created
	int lightCounter;
};

/// Loads selected lighting into the active shader program
void LoadLighting(Lighting * lighting, Shader * shader);

#endif
