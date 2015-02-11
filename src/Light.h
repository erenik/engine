/// Emil Hedemalm
/// 2014-01-19
/// Simple structure for a light (any kind)

#ifndef LIGHT_H
#define LIGHT_H

#include "MathLib.h"
#include "Util.h"

/// Light types should maybe be definable or something, hm..
namespace LightType {
enum LightTypes {
	NULL_TYPE,
	POSITIONAL, POINT = POSITIONAL,	/// Point and position light are the same thing~
	DIRECTIONAL, 
	SPOTLIGHT,

	LIGHT_TYPES,
};
};

class Entity;
class Message;

/** Struct for handling a single light source. */
class Light
{
	friend class Lighting;
public:
	/// Default constructor
	Light();
	Light(Lighting * lighting);
	Light(const Light & otherLight);
	~Light();
	void Nullify();

	/// Opens a dedicated editor window for this light. Assumes a valid LightEditor.gui is available in the UI directory.
	void OpenEditorWindow();
	void CloseEditorWindow();
	// For interaction with UI as well as scripting.
	void ProcessMessage(Message * message);
	// Updates UI as necessary
	void OnPropertiesUpdated();

	/// Writes to file stream.
	void WriteTo(std::fstream & file);
	/// Reads from file stream.
	void ReadFrom(std::fstream & file);

	/// Ambient light intensity. Consider stop using?
	Vector4f ambient;
	/// Diffuse light intensity.
	Vector4f diffuse;
	/// Specular light intensity.
	Vector4f specular;
	/// Position in the world. Fourth parameter <1 or 0> defines ´directional/positional´ model.
	Vector3f position;
	/// When attached to an entity.
	Vector3f relativePosition;
	/// Light type
	int type;
	/// Distance attenuations for the light. Constant, linear and quadratic.
	Vector3f attenuation;
	/** If it is currently rendered in the scene for the current camera.
		A light that is not active can be considered dead, and may thus be replaced at any time!
	*/
	bool currentlyActive;
	/// Default false.
	bool castsShadow;
	/// Last time we changed any of it's properties.
	long lastUpdate;
	/// Returns name of the light
	String Name() const { return name; };	
	/// Sets name of the light
	void SetName(const String newName);

	// Spotlight specific attributes:
	/// Defines the direction for spotlights, in global coordinates.
	Vector3f spotDirection;
	/// When attached to an entity.
	Vector3f relativeSpotDirection;
	/** Defines the exponent applied after calculating the light intensity (cosine of angel from main light direction). 
		Defines intensity as the light goes away from centre to the cutoff. 
	*/
	int spotExponent;
	/// Defines the spot lights area of influence by determining the radius (in degrees) out from the spotDirection.
	float spotCutoff;
	
	/// Entity owner.
	Entity * owner;
	/// Extra data pointer. For dynamic lights this pointer will refer to the entity owning it.
	void * data;

	// For dynamic lights. Default false.
	bool registeredForRendering;
private:
	// Lighting setup it belongs to.
	Lighting * lighting;
	/// Name of the light
	String name;
};

#endif