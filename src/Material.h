#ifndef MATERIAL_H
#define MATERIAL_H

/// A structure containing information about a specific material. All arrays are ordered Red, Green, Blue & Alpha in that order.
struct Material {
	/// Default constructor, sets ambient RGB to 0.2, diffuse RGB to 0.8, specular RGB to 0, emission RGB to 0 and emission RGB to 0. All alpha values are set to 1.
	Material();

	/// Ambient light reception.
	float ambient[4];
	/// Diffuse light reception. The alpha parameter in the diffuse array affects the actual opacity of the Entity when rendering.
	float diffuse[4];
	/// Specular light reception. Shininess also affects how the specular highlights are calculated.
	float specular[4];
	/// Light emission. 
	float emission[4];
	/// Specular shininess. Affects how refracted light behaves together with the specular values.
	int shininess;

	/// Sets all ambient values to 1.0 and all other values (except alpha) to 0. This is used by meshes that lack normal-coordinates.
	void justAmbient();
};


#endif