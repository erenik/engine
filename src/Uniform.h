#ifndef UNIFORM_LIGHT_H
#define UNIFORM_LIGHT_H

#include "Graphics/OpenGL.h"

/// Struct containing uniform IDs for a material in the shader
struct UniformMaterial{
	/// Ambient light absorption.
	GLuint ambientVec4;
	/// Diffuse light absorption.
	GLuint diffuseVec4;
	/// Specular light absorption.
	GLuint specularVec4;
	/// Specular shininess/roughness.
	GLuint shininessInt;
};


/// Struct containing uniform IDs for a light in the shader
struct UniformLight{
	/// Specifies if the light is active at all
	GLuint activeBool;
	/// Ambient light intensity.
	GLuint ambientVec4;
	/// Diffuse light intensity.
	GLuint diffuseVec4;
	/// Specular light intensity.
	GLuint specularVec4;

	/// New position for xyz
	GLuint positionVec3;
	/// OBSOLETE: Position in the world. Fourth parameter (w) decides if it is directional (0) or not (!0). TODO: Remove
	GLuint dirOrPosVec4;
	/// Attenuation factors: constant, linear and quadratic. All stored in one vec3
	GLuint attenuationVec3;
	/// Type, 1 = Positional, 2 = Directional, 3 = Spotlight
	GLuint typeInt;

	/// Spotlight direction in global coordinates (XYZ)
	GLuint spotDirectionVec3;
	/// Spotlight cutoff in degrees from center of light direction.
	GLuint spotCutoffFloat;
	/// Spotlight exponent applied relative to cutoff and center of light cone.
	GLuint spotExponentInt;
};

#endif
