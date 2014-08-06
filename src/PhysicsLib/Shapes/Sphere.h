/// Emil Hedemalm
/// 2014-08-06
/// Simple shapes.

#ifndef SPHERE_H
#define SPHERE_H

class Sphere 
{
public:
#define DEFAULT_SECTIONS	8
	Sphere() { radius = 1.0f; sections = DEFAULT_SECTIONS; };
	Sphere(float radius, Vector3f position = Vector3f());
	float radius;
	Vector3f position;
	int sections;	// For if generating/rendering custom spheres or doing tests using it for some stupid reason..
};

#endif
