// Author: Emil Hedemalm
// Date: 2016-06-20
// Name: Deferred lighting n shadow mapping
#version 120

// Uniforms
// View matrix provided by the client.
uniform mat4 viewMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// Projection matrix provided by the client.
uniform mat4 projectionMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

/// Sample depth-map so that the fragments land correctly, not obscuring SkyBox, etc.
uniform sampler2D depthMap;

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2.
attribute vec3 in_Position;
attribute vec2 in_UV;

// Output data for the fragment shader
varying vec2 UV_Coord;		// Just passed on
varying vec3 normal;		// Normal coordinates
varying vec3 vecToEye;		// Vector from eye to the vertex (later fragment)
varying vec4 v_Tangent;

varying vec3 constantColor;

void main()
{
	constantColor = vec3(0,0,0);
	// Calculate matrices
	mat4 mvp = projectionMatrix;
	float zDepth = texture2D(depthMap, UV_Coord).x;
	// Multiply mvp matrix onto the vertex coordinates.
	gl_Position = vec4(in_Position, 1);	
	UV_Coord = in_UV;
}


