// Author: Emil Hedemalm
// Date: 2015-02-11
#version 120

// Uniforms
// View matrix provided by the client.
uniform mat4 viewMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// Projection matrix provided by the client.
uniform mat4 projectionMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// Normal matrix to be applied for all normals! D:
uniform mat4 normalMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// Position of eye in the world.
uniform vec4 eyePosition	= vec4(0.0, 5.0, 0.0, 0.0);

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2, Tangents 3
attribute vec3 in_Position;

// Output data for the fragment shader
varying vec3 v_normal;		// Interpollated coordinates that have been transformed to view space
varying vec2 UV_Coord;		// Just passed on
varying vec3 vertexPosition;		// World coordinates of the vertex
varying vec3 vecToEye;		// Vector from vertex to eye
varying vec4 v_Tangent;		// Face-tangent, to be interpollated?

// Just pass through positions after transforming them properly.
void main()
{			
	// Calculate matrices
	mat4 vp = projectionMatrix * viewMatrix; // * modelMatrix;
	// Multiply mvp matrix onto the vertex coordinates.
	vec4 viewProjectionPosition = vp * vec4(in_Position, 1);
	vec4 viewPosition = viewMatrix * vec4(in_Position, 1);	
	vec4 glPosition = viewProjectionPosition;
	glPosition.z = glPosition.w;
	/// Just pass along the world-position... or possibly normalize it?
	vertexPosition = normalize(in_Position);
	gl_Position = glPosition;	
}

