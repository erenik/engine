// Author: Emil Hedemalm
// Date: 2012-10-29
#version 120

// Uniforms
// 2D Texture texture
uniform sampler2D baseImage;

// Input data from the fragment shader
in vec3 normal;		// Interpollated coordinates that have been transformed to view space
in vec2 UV_Coord;	// Just passed on
in vec3 worldCoord;	// World coordinates of the fragment
in vec3 vecToEye;	// Vector from vertex to eye
in vec3 position;

void main(){
	/// Just paint normals as RGB ^^
	gl_FragColor = vec4(0.5,0.5,0.5,1);
	gl_FragColor += vec4(normal.x, normal.y, normal.z, 1);
	return;
}
 
