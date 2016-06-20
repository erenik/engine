/// Author: Emil Hedemalm
/// Date: 2012-10-29
/// Only Diffuse + Emissive (if any)
#version 120

// Uniforms
// Model matrix provided by the client.
uniform mat4 modelMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);	 
// View matrix provided by the client.
uniform mat4 viewMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// Projection matrix provided by the client.
uniform mat4 projectionMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2.
attribute vec3 in_Position;
attribute vec2 in_UV;

// Output data for the fragment shader
varying vec2 UV_Coord;		// Just passed on
varying vec3 position;

void main(){
	// Calculate matrices
	mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
	
    // Multiply mvp matrix onto the vertex coordinates.
    vec4 posPos = mvp * vec4(in_Position, 1);
	position = posPos.xyz;
    gl_Position = posPos;
    
	// Just pass on the UV-coordinates ^^
	UV_Coord = in_UV;
}

