// Author: Emil Hedemalm
// Date: 2014-09-18
#version 120

// Uniforms
// Model, view and projection matrices provided by the client.
// uniform mat4 modelMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
uniform mat4 viewMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
uniform mat4 projectionMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2.
attribute vec3 in_VertexPosition; // Position of vertex.
attribute vec4 in_ParticlePositionScale; // Position of the particle and it's scale in the w-component.
attribute vec4 in_Color;
attribute vec2 in_ParticleLifeTimeDuration; //Life time (total) and duration (current)

// Output data for the fragment shader
varying vec2 UV_Coord;		// Just passed on
varying vec4 position;
varying vec4 particleColorMultiplier;
varying vec2 lifeTimeDuration;

// https://www.opengl.org/sdk/docs/man/html/glDrawArraysInstanced.xhtml
// http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/

void main()
{
	vec4 cameraRightWorldSpace = normalize(viewMatrix * vec4(1,0,0,0));
	vec4 cameraUpWorldSpace = normalize(viewMatrix * vec4(0,1,0,0));

	lifeTimeDuration = in_ParticleLifeTimeDuration;
	
	// gl_InstanceID
	// Calculate matrices
	mat4 mvp = projectionMatrix * viewMatrix;
	
	float scale = in_ParticlePositionScale.w;
	vec2 billboardSize = vec2(scale, scale);
	UV_Coord = in_VertexPosition.xy + vec2(0.5, 0.5);
	
	vec4 particleCenter_worldSpace = vec4(in_ParticlePositionScale.xyz, 1);
	vec4 vertexPosition_worldSpace = vec4(particleCenter_worldSpace.xyz, 1)
		+ cameraRightWorldSpace * in_VertexPosition.x * billboardSize.x
		+ cameraUpWorldSpace * in_VertexPosition.y * billboardSize.y;
	
	mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
	position = viewProjectionMatrix * vertexPosition_worldSpace;	
    gl_Position = position;
	particleColorMultiplier = in_Color;	
}

