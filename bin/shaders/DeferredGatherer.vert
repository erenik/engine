// Emil Hedemalm
// 2016-06-20
// Shader that gathers data for the deferred lighting. V2.
#version 120

// Uniforms
// Model matrix provided by the client.
uniform mat4 modelMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);	 
// View matrix provided by the client.
uniform mat4 viewMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// Projection matrix provided by the client.
uniform mat4 projectionMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// Matrix to be multiplied for the normals (for rotations primarily)
uniform mat4 normalMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// Position of eye in the world.
uniform vec4 eyePosition	= vec4(0.0, 5.0, 0.0, 0.0);

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2.
in vec3 in_Position;
in vec2 in_UV;
in vec3 in_Normal;
in vec4 in_Tangent;

// Output data for the fragment shader
varying vec3 worldCoord;
varying vec2 UV_Coord;		// Just passed on
varying vec3 normal;		// Normal coordinates
varying vec3 vecToEye;		// Vector from eye to the vertex (later fragment)
varying vec4 v_Tangent;

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

	// Transform the normal to world-coordinates and then normalize it.
	normal = normalize(mat3(normalMatrix) * in_Normal).xyz;
	
	mat3 linearPartOfMM = mat3(modelMatrix);
	v_Tangent.xyz = normalize(linearPartOfMM * in_Tangent.xyz); //normalize(normalMatrix * in_Tangent);
}


