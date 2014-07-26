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
// Normal matrix to be applied for all normals! D:
uniform mat4 normalMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2.
attribute vec3 in_Position;
attribute vec2 in_UV;
attribute vec3 in_Normal;

// Output data for the fragment shader
varying vec2 UV_Coord;		// Just passed on
varying vec3 v_normal;		// Pass on and interpollate normals
varying vec3 position;

void main(){
	// Calculate matrices
	mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
	// Multiply mvp matrix onto the vertex coordinates.
	
	vec4 pos = mvp * vec4(in_Position, 1);
	position = pos.xyz;
	gl_Position = pos;
	// Just pass on the Normal-coordinates~
	// Normalize the normal after transforming it using the model's normal matrix (which takes into consideration pos/scale/rot)
	v_normal = normalize(mat3(normalMatrix) * in_Normal).xyz;
}
