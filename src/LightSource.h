

#ifndef LIGHT_H
#define LIGHT_H

#include "Node.h"

/** A custom lightsource class, storing all data needed for a light in OpenGL.
	All arrays store color in the usual Red, Green, Blue & Alpha order.
*/
class LightSource : public Node {
	
public:
	/** Default constructor. 
		Sets ambient to (0,0,0,1), diffuse and specular to (1,1,1,1), the constant attenuation factor to 1 and the remaining attenuation factors to 0.
		Sets the initial position to (0,2,0,1), making it slightly above origo, and not treated as a directional light source.
	*/
	LightSource();
	/** Activates the light source and renders any applicable children as normal.
		Deactivates it's light source and renders the sibling if it has any afterwards.
		If RENDER_LIGHT_POSITION is specified in the AppState, the light source will be rendered using 7 GL_POINTS at it's current location.
	*/
	void Render();
	/** Renders the position of the light source using 7 GL_POINTS, one in the center and the rest each 1 carteesian unit away from the source.
	*/
	void renderPosition(GraphicsState &state);

	/// Is it active?
	bool active;
	
	/// Ambient light intensity.
	float ambient[4];
	/// Diffuse light intensity.
	float diffuse[4];
	/// Specular light intensity.
	float specular[4];
	/// Position in the world.
	Vector4f position;
	
	/// Type, 0 = directional, 1 = positional
	int type;	
	
	/// Constant Attenuation is the constant factor divided to all light intensities.
	float constantAttenuation;
	/// Linear Attenuation is the factor multiplied with the distance to the Entity that is then divided to all light intensities.
	float linearAttenuation;
	/// Quadratic Attenuation is the factor that is multiplied twice with the distance to the Entity that is then divided to all light intensities.
	float quadraticAttenuation;
private:

};




#endif