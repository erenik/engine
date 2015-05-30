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
// Position of eye in the world.
uniform vec4 eyePosition	= vec4(0.0, 5.0, 0.0, 0.0);

// Replacing view- and projection matrices. Hopefully...
uniform mat4 viewProjectionMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2, Tangents 3
attribute vec3 in_Position;
attribute vec2 in_UV;
attribute vec3 in_Normal;
attribute vec4 in_Tangent; // Tangent XY in XY and CoTangent XY in ZW?

// Output data for the fragment shader
varying vec3 v_normal;		// Interpollated coordinates that have been transformed to view space
varying vec2 UV_Coord;		// Just passed on
varying vec3 position;		// World coordinates of the vertex
varying vec3 vecToEye;		// Vector from vertex to eye
varying vec4 v_Tangent;		// Face-tangent, to be interpollated?

//// o.o for instancing.
attribute mat4 in_InstanceModelMatrix;
attribute mat4 in_InstanceNormalMatrix;

/// Toggles instancing. If instancing, uses attributes instead of uniforms.
uniform int instancingEnabled = 0;

// Just pass through all data after transforming them properly.
void main()
{	
	// Depends on instancing.
	mat4 currentModelMatrix;
	mat4 currentNormalMatrix;
	if (instancingEnabled == 1)
	{
		currentModelMatrix = in_InstanceModelMatrix;
		currentNormalMatrix = in_InstanceNormalMatrix;
	}
	/// Individually.
	else 
	{
		currentModelMatrix = modelMatrix;
		currentNormalMatrix = normalMatrix;
	}

	mat3 linearPartOfMM = mat3(currentModelMatrix);
	
	v_Tangent.xyz = linearPartOfMM * in_Tangent.xyz; //normalize(normalMatrix * in_Tangent);
	
	// Calculate matrices
	mat4 mvp = viewProjectionMatrix * currentModelMatrix;
//	mat4 mvp = projectionMatrix * viewMatrix * currentModelMatrix;

	// Multiply mvp matrix onto the vertex coordinates.
	gl_Position = mvp * vec4(in_Position, 1);
	// Multiply the model matrix onto the coordinate to get world-space coordinates.
	position = (currentModelMatrix * vec4(in_Position, 1)).xyz;
	
	/// Multiply with transpose of inverse modelView matrix
//	mat4 inverseModel = inverse(currentModelMatrix);
//	mat4 normalMatrix = transpose(inverseModel);
	// Normalize the normal.
	v_normal = normalize(mat3(currentNormalMatrix) * in_Normal).xyz;
//	normal = in_Normal;

	// Just pass on the UV-coordinates ^^
	UV_Coord = in_UV;
	// Calculate vector to eye from vertex.
	vecToEye = normalize((eyePosition.xyz - position.xyz));
}
