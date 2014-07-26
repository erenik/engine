// Author: Emil Hedemalm
// Date: 2012-10-29
#version 120

// Uniforms
// No matrices needed for the lighting ^.^

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2.
in vec3 in_Position;
in vec2 in_UV;

// Output data for the fragment shader
varying vec2 UV_Coord;		// Just passed on
varying vec3 Position;

void main(){
	// Just pass it on..
	//Position = in_Position;
	gl_Position = vec4(in_Position, 1);
	// Just pass on the UV-coordinates ^^
	UV_Coord = in_UV;
}

