// Author: Emil Hedemalm
// Date: 2012-10-29
// Renders wireframe of the mesh in specified color ^^
#version 120

// Uniforms
uniform vec4 primaryColorVec4 = vec4(1,1,1,0.5);
uniform float rainbowXYZFactor = 0.1f;

// Input data from the fragment shader
varying vec3 normal;		// Interpollated coordinates that have been transformed to view space
varying vec3 worldCoord;	// World coordinates of the fragment
varying vec3 position;

void main(){
	gl_FragColor = primaryColorVec4;
	
	gl_FragColor += vec4(position.x, position.y, position.z, 0) * rainbowXYZFactor;
//	gl_FragColor.w += 0.5f;
//	gl_FragColor = vec4(2,1,1,1);
	return;
}
 
