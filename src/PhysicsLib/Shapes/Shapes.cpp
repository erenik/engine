/// Emil Hedemalm
/// 2014-08-11
/// Simple shapes, old file..


#include "Shapes.h"
#include <cassert>

#include "Ngon.h"
#include "Line.h"
#include "Sphere.h"

Ngon::Ngon(){
	assert(false);
}

/// Sphere Initializer
Sphere::Sphere(float radius, Vector3f position /* = Vector3f()*/ )
: radius(radius), position(position), sections(DEFAULT_SECTIONS)
{
}





