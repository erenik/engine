// Author: Emil Hedemalm
// Date: 2014-09-18
#version 120

// Uniforms
// Model, view and projection matrices provided by the client.
uniform mat4 viewProjectionMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

uniform vec3 cameraRightWorldSpace = vec3(1,0,0);
uniform vec3 cameraUpWorldSpace = vec3(0,1,0);

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2.
attribute vec3 in_Position;
attribute vec2 in_UV;
attribute vec3 in_Normal;

attribute vec4 in_ParticlePositionScale; // Position of the particle and it's scale in the w-component.
attribute vec4 in_Color;
attribute vec4 in_ParticleLifeTimeDurationScale; //Life time (total) and duration (current)

// Output data for the fragment shader
varying vec2 v_UV_Coord;		// Just passed on
varying vec3 v_w_normal;
varying vec3 v_w_position;
varying vec4 color;
varying vec2 lifeTimeDuration;

varying vec3 debugColor;

// https://www.opengl.org/sdk/docs/man/html/glDrawArraysInstanced.xhtml
// http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/

void main()
{
	debugColor = vec3(0,0,0);
	vec2 billboardSize = in_ParticleLifeTimeDurationScale.zw;
	// Need inverted, and that is already done in the engine, made into uniforms instead!
	lifeTimeDuration = in_ParticleLifeTimeDurationScale.xy;
	// Pass along data.
	v_UV_Coord = in_UV;
	v_w_normal = in_Normal;
	
	// gl_InstanceID
	// Calculate matrices
	mat4 mvp = viewProjectionMatrix;
		
	// Scale with scale.
	// Translate with position.
	vec3 position = in_Position;
	position *= in_ParticleLifeTimeDurationScale.z;
	position += in_ParticlePositionScale.xyz;	
	v_w_position = position.xyz;
	gl_Position = (mvp * vec4(position, 1));
	
	color = in_Color;

//	debugColor.x = 1;
}

