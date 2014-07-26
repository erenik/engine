// Author: Emil Hedemalm
// Date: 2012-10-29
#version 120

// For bit-wise operations!
#extension GL_EXT_gpu_shader4 : enable

// Uniforms
// Position of eye in the world.
uniform vec4 eyePosition	= vec4(0.0, 5.0, 0.0, 0.0);	
// Color applied to all stuff in the final.
uniform vec4 primaryColorVec4 = vec4(1.0, 1.0, 1.0, 1.0);

// For if-checking for applying normal maps, specular maps, etc.
#define DIFFUSE_MAP		0x0000001
#define SPECULAR_MAP	0x0000002
#define NORMAL_MAP		0x0000004
uniform int texturesToApply = 0;

// Material statistics
uniform vec4	materialAmbient		= vec4(0.2, 0.2, 0.2, 1.0);
uniform vec4	materialDiffuse		= vec4(0.8, 0.8, 0.8, 1.0);
uniform vec4	materialSpecular	= vec4(1.1, 1.1, 1.1, 1.0);
uniform int		materialShininess	= 8;

// 2D Texture texture
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

// Input data from the fragment shader
varying vec3 v_normal;		// Interpollated coordinates that have been transformed to view space
varying vec2 UV_Coord;	// Just passed on
varying vec3 position;	// World coordinates of the fragment
varying vec3 vecToEye;	// Vector from vertex to eye
varying vec4 v_Tangent;	// Tangent!

void main(){
	float depth = gl_FragCoord.z;
	gl_FragDepth = depth; //depth.x;
	
	gl_FragColor = vec4(1,1,1,1);
	gl_FragColor = primaryColorVec4;
	return;
}
