// Author: Emil Hedemalm
// Date: 2012-10-29
#version 120

// Uniforms
// Model matrix provided by the client.
uniform mat4 modelMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);	 
// View matrix provided by the client.
uniform mat4 viewMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// Projection matrix provided by the client.
uniform mat4 projectionMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// Position of eye in the world.
uniform vec4 eyePosition	= vec4(0.0, 5.0, 0.0, 0.0);

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2.
in vec3 in_Position;
in vec2 in_UV;

// Output data for the fragment shader
varying vec2 UV_Coord;		// Just passed on
varying vec3 vecToEye;		// Vector from eye to the vertex (later fragment)
varying vec3 worldCoord;

void main(){
	// Calculate matrices
	mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
	// Multiply mvp matrix onto the vertex coordinates.
	gl_Position = mvp * vec4(in_Position, 1);
	// Just pass on the UV-coordinates ^^
	UV_Coord = in_UV;

	// Multiply the model matrix onto the coordinate to get world-space coordinates.
	worldCoord = (modelMatrix * vec4(in_Position, 1)).xyz;

	// Calculate vector to eye from vertex.  DO NOT NORMALIZE since we might want the distance ^^
	vecToEye = (eyePosition.xyz - worldCoord);
}

/*
#version 150

// Uniforms
// Model matrix provided by the client.
uniform mat4 modelMatrix;	 
// View matrix provided by the client.
uniform mat4 viewMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// Projection matrix provided by the client.
uniform mat4 projectionMatrix;	
// Position of eye in the world.
uniform vec4 eyePosition	= vec4(0.0, 5.0, 0.0, 0.0);

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2.
in vec3 in_Position;
in vec2 in_UV;
in vec3 in_Normal;

// Output data for the fragment shader
out vec3 normal;		// Interpollated coordinates that have been transformed to view space
out vec2 UV_Coord;		// Just passed on
out vec3 worldCoord;	// World coordinates of the vertex
out vec3 vecToEye;		// Vector from vertex to eye

void main(void) {
	
	// Calculate matrices
	mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;

	// Multiply mvp matrix onto the vertex coordinates.
	gl_Position = mvp * vec4(in_Position, 1);
	// Multiply the model matrix onto the coordinate to get world-space coordinates.
	worldCoord = (modelMatrix * vec4(in_Position, 1)).xyz;
	
	/// Multiply with normalized transpose of modelView matrix
	mat4 normalMatrix = transpose(inverse(modelMatrix));
	// Normalize the normal.
	normal = normalize(normalMatrix * vec4(in_Normal, 1)).xyz;

	// Just pass on the UV-coordinates ^^
	UV_Coord = in_UV;
	// Calculate vector to eye from vertex.
	vecToEye = normalize((eyePosition.xyz - worldCoord));
}
*/
 
