// Author: Emil Hedemalm
// Date: 2012-10-29
#version 120

// Uniforms
// 2D Texture texture
uniform sampler2D baseImage;

// Input data from the fragment shader
varying vec3 v_normal;		// Interpolated coordinates that have been transformed to view space
varying vec2 UV_Coord;	// Just passed on
varying vec3 worldCoord;	// World coordinates of the fragment
varying vec3 vecToEye;	// Vector from vertex to eye
varying vec3 position;

void main(){
	/// Just paint normals as RGB ^^
	gl_FragColor = vec4(0.5,0.5,0.5,1);
	
	vec3 normal = normalize(v_normal);
	gl_FragColor += vec4(normal.xyz, 1);
}
 
