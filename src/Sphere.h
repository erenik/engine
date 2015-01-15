/// Emil Hedemalm
/// 2014-12-05
/// Creates/maintains (parts of) a Sphere

#ifndef SPHERE_H
#define SPHERE_H

#include "Mesh/Mesh.h"

class Sphere : public Mesh 
{
public:
	Sphere();
	Sphere(float radius, Vector3f position = Vector3f());
	virtual ~Sphere();

	/** Creates a "quadratic" sphere-segment, 
		using given width and height (both in radians) from the middle of X and Y respectively.
		Assigns UV-coordinates automatically in a quadratic projection sense.

		Width (Size.x) should be between 0 and 2 PI, while height (Size.y) should be between 0 and PI.
	*/
	static Sphere * CreateSegmentFromEquator(Vector2f size, Vector2i segments, float offsetX, bool invertTexUCoords);

	float radius;
	Vector3f position;
	int sections;	// For if generating/rendering custom spheres or doing tests using it for some stupid reason..

	/// For when creating.
	bool invertTexUCoords;
protected:

	void Generate();
	/// Dictates resolution (amount of quads).
	Vector2i segments;
	/// For parts of a sphere, dictates the degrees used to generate the parts of it.
	Vector2f size;

	float offsetX;

};

#endif
