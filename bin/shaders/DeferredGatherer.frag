// Emil Hedemalm
// 2016-06-20
// Shader that gathers data for the deferred lighting. V2.
#version 330

// Uniforms
// 2D Texture texture. Set these to -1 if you are not going to use them!
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;
uniform sampler2D emissiveMap;

// Matrix to be multiplied for the normals (for rotations primarily)
uniform mat4 normalMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

// Input data from the fragment shader
in vec3 normal;		// Interpollated coordinates that have been transformed to view space
in vec2 UV_Coord;	// Just passed on
in vec3 worldCoord;	// World coordinates of the fragment
in vec3 vecToEye;	// Vector from vertex to eye
in vec3 position;
in vec3 specular;	// Spec spec.
in vec4 v_Tangent; 	// Wosh.

/// Explicit out locations for the destination FrameBuffer 
layout (location = 0) out vec4 diffuseOut;	// Diffuse, normal, position, in that order!
layout (location = 1) out vec4 normalOut;
layout (location = 2) out vec4 positionOut;
layout (location = 3) out vec4 specularOut;
layout (location = 4) out vec4 emissiveOut;

/*
layout (location = 4) out vec4 tangentOut;	// For normalMapping
layout (location = 5) out vec4 normalMapOut; // - || -
layout (location = 6) out vec4 pickingOut;	// For pickling..!
layout (location = 7) out vec4 depthOut; 	// Depth should probably not be layouted since it is already bound to be the depth texture?
*/

uniform int pickingID = 1;
uniform vec4 primaryColorVec4 = vec4(1,1,1,1);

// For if-checking for applying normal maps, specular maps, etc.
#define DIFFUSE_MAP		0x0000001
#define SPECULAR_MAP	0x0000002
#define NORMAL_MAP		0x0000004
// uniform int texturesToApply = 0; // DIFFUSE_MAP | SPECULAR_MAP | NORMAL_MAP;


void main(){
	// Texture image data. This will be the base for the colors.
	vec3 white = vec3(1,1,1);

	/// Assume eye is at 0,0,0 for now
	float distance = length(vecToEye);
	float intensity = 1.0f;
	intensity = 1.0f / (1.0f + distance);

	// DO NOTE: Alpha has to be Positive for the value to be printed out to the texture AT ALL! 
	// It defines probably the ratio at which it is sent down the pipeline. o-o;;
	normalOut = vec4((normalize(normal)).xyz, 1);
	if (true)
		diffuseOut = texture2D(diffuseMap, UV_Coord) * primaryColorVec4;
	else
		diffuseOut = vec4(1.3,0.3,0.3,1.0) * primaryColorVec4;
		
	// Apply Specularus?
	if (true){
		specularOut = vec4(texture2D(specularMap, UV_Coord).xyz, 1) * primaryColorVec4;
	}
	else
		specularOut = vec4(0.2,0.2,0.2,1.0) * primaryColorVec4;
	
/*	
	// Apply Tangentus?
	if (false){
		tangentOut = vec4(v_Tangent.xyz, 1);
		normalMapOut = texture2D(normalMap, UV_Coord);
	}
	else {
		tangentOut = vec4(0,0,0,1);
		normalMapOut = vec4(0.5,0.5,1.0,1.0);
	}
	*/	
	
	if (true)
		emissiveOut = vec4(texture2D(emissiveMap, UV_Coord).xyz, 1) + vec4(1,0,0,0);
	
//	depthOut = vec4(intensity, intensity, intensity, 1);
	positionOut = vec4(worldCoord, 1);
	
//	pickingOut = vec4(pickingID, 1, 1, 1);
	return;
}
