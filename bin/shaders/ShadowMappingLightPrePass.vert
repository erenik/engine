// Author: Emil Hedemalm
// Date: 2015-02-20
// Very light-weight shadow mapping shader.
#version 120

// Uniforms
// Model matrix provided by the client.
uniform mat4 modelMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// View matrix provided by the client.
uniform mat4 viewMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// Projection matrix provided by the client.
uniform mat4 projectionMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2, Tangents 3
attribute vec3 in_Position;

//// o.o for instancing.
attribute mat4 in_InstanceModelMatrix;

/// Toggles instancing. If instancing, uses attributes instead of uniforms.
uniform int instancingEnabled = 0;

// Just pass through positions after transforming them properly.
void main()
{	
	mat4 mvp;
	/// Instancing.
	if (instancingEnabled == 1)
	{
		mvp = projectionMatrix * viewMatrix * in_InstanceModelMatrix;	
//		mvp = projectionMatrix * viewMatrix * modelMatrix;
	}
	/// Individually.
	else 
	{
		mvp = projectionMatrix * viewMatrix * modelMatrix;		
	}
	gl_Position = mvp * vec4(in_Position, 1);
}

