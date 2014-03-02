
#ifndef LIGHTING_H
#define LIGHTING_H

struct GraphicsState;

#include "Light.h"
struct Light;

#define MAX_LIGHTS 128

/** A structure for keeping track of currently active lights in a scene
	It contains mostly pointers since it is an encapsulating object for passing data between functions.
*/
class Lighting {
	/// Loads selected lighting into the active shader program
	friend bool LoadLighting(Lighting * lighting, GraphicsState &state);
#define GetTime clock
public:

	/// Default constructor, sets all light pointers and other variables to 0/NULL
	Lighting();
	~Lighting();
	// Verifies that some basic looks good.
	void VerifyData() const;
	/// Copy constructor ^^
	Lighting(const Lighting& lighting);
	/// Assignment operator...
	const Lighting * operator = (const Lighting & otherLighting);

	/// Adds light to the lighting, returns it's index on success, -1 on failure. o-o
	int Add(Light & light);
	/// Removes light from the lighting, returns it's old index on success, -1 on failure. o-o
	int Remove(Light & light);
	/// Creates a default setup of 3-4 lights for testing purposes.
	void CreateDefaultSetup();
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
	Vector4f GetAmbient() { return Vector4f(global_ambient); };
	/// Creates a new light source, adding it to the array. Returns the newly created light's index and sets it as the currently editable light.
	int CreateLight();
	/// Returns a pointer to the active light. USE WITH CAUTION.
	Light * GetLight() { return activeLight; };
	/// Returns a pointer to selected light. USE WITH CAUTION.
	Light * GetLight(int index);
	/// Deletes active light source. Returns false if no light is selected.
	bool DeleteLight();
	/// Deletes all light sources contained within.
	int DeleteAllLights();
	/// Selects indexed light. Returns it's index upon success, -1 upon failure.
	int SelectLight(int index);
	/// Selects light by name. Returns it's index upon success, -1 upon failure.
	int SelectLight(const char * name);
	/// Sets diffuse values for active light
	void SetDiffuse(float r, float g, float b, float a = 1.0f);
	/// Sets specular values for active light
	void SetSpecular(float r, float g, float b, float a = 1.0f);
	/// Sets diffuse & specular values for active light
	void SetColor(float r, float g, float b, float a = 1.0f);
	/// Sets attenuation factors for the active light
	void SetAttenuation(float constant, float linear, float quadratic);
	/// Sets position for the active light
	void SetPosition(float x, float y, float z);

	/// Sets spotlight direction in world coordinates
	void SetSpotDirection(float x, float y, float z);
	/// Sets spotlight cutoff in degrees and exponent for edge-fading.
	void SetSpotCutoffExponent(float cutoff, int exponent);

	/// Sets light-type for the active light
	void SetType(int type);
	/// Sets new name for the active light
	void SetName(const char * newName);
	/// Gets name of selected light.
	const char * GetName(int index);

	/// Returns current amount of active lights
	int ActiveLights() const { return activeLights; };
	
	/// Writes to file stream.
	void WriteTo(std::fstream & file);
	/// Reads from file stream.
	void ReadFrom(std::fstream & file);
private:
	/// Pointer to array of 4 floats (rgba)
	Vector4f global_ambient;
	/// Array of [MAX_LIGHTS] lights.
	Light * light[MAX_LIGHTS];
	/// Number of active lights in the above array.
	int activeLights;
	/// Last time we changed any of the internal parameters for this current lighting
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
bool LoadLighting(Lighting * lighting, GraphicsState &state);

#endif
