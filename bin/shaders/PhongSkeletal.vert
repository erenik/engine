// Author: Emil Hedemalm
// Date: 2014-09-29
#version 130

// Standard matrices
uniform mat4 modelMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
uniform mat4 viewMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
uniform mat4 projectionMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
uniform mat4 normalMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

// Position of eye in the world.
uniform vec4 eyePosition	= vec4(0.0, 5.0, 0.0, 0.0);

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2, Tangents 3
attribute vec3 in_Position;
attribute vec2 in_UV;
attribute vec3 in_Normal;
attribute vec4 in_Tangent;

// Indices for bones! Max 4 bones per vertex is the limit... for now.
attribute ivec4 in_BoneIndices;
// Weights for said bones.
attribute vec4 in_BoneWeights;

// Uniform for the sampling texture in which we will place all bones transformation matrices for the current frame.
uniform sampler2D boneSkinningMatrixMap;

// Output data for the fragment shader
varying vec3 v_normal;		// Vertex normal in view space
varying vec2 UV_Coord;		// Just passed on
varying vec3 position;		// World coordinates of the vertex
varying vec3 vecToEye;		// Vector from vertex to eye
varying vec4 v_Tangent;		// Face-tangent

// Outputed to the fragment shader and added at the end. Default to 0,0,0,0 if not debugging anything!
varying vec4 debugColor;

void main()
{
	debugColor = vec4(0,0,0,1);
	mat3 linearPartOfMM = mat3(modelMatrix);
	
	vec3 modelSpacePosition = vec3(0,0,0);	
		
	int boneIndex;
	float boneWeight;
	float totalWeights = 0;
	for (int i = 0; i < 4; ++i)
	{
		if(i == 0)
		{
			boneIndex = in_BoneIndices.x;
			boneWeight = in_BoneWeights.x;
		}
		else if (i == 1)
		{
			boneIndex = in_BoneIndices.y;
			boneWeight = in_BoneWeights.y;
		}
		else if (i == 2)
		{
			boneIndex = in_BoneIndices.z;
			boneWeight = in_BoneWeights.z;
		}
		else if (i == 3)
		{
			boneIndex = in_BoneIndices.w;
			boneWeight = in_BoneWeights.w;
		}
		if (boneWeight < 0)
			continue;
		
		// Apply weight-change to debug.
		boneWeight *= 1;
		
		
		totalWeights += boneWeight;

		// Visualize bone-indices influencing the joints.
		bool visualizeIndices = false;
		if (visualizeIndices)
		{
			float r,g,b;
			r = 1 - abs(boneIndex - 0)/10;
			g = 1 - abs(boneIndex - 25)/10;
			b = 1 - abs(boneIndex - 50)/10;
			debugColor += vec4(r,g,b,1) * 0.25;
		}
		
		// Get matrix, combined of the inverse bind pose matrix and the current transform matrix of the bone.
		mat4 skinningMatrix;
		int ones = 0;
		for (int c = 0; c < 4; ++c)
		{
			for (int r = 0; r < 4; ++r)
			{
				// Convert to local co-ordinates.. maybe?
				float x = (c * 4 + r + 0.5) / 16.0;
				float y = (boneIndex + 0.5) / 55.0;
				vec4 texData = texture2D(boneSkinningMatrixMap, vec2(x, y));
				skinningMatrix[c][r] = texData.x;
				
				float matrixElementValue = skinningMatrix[c][r];
			}
		}
				
		vec3 vertexPositionPart = (skinningMatrix * vec4(in_Position, 1)).xyz;
		
		modelSpacePosition += vertexPositionPart * boneWeight;
		
	}
	
	// Add position based on weight remainder
	modelSpacePosition += in_Position * (1 - totalWeights);	
	
	v_Tangent.xyz = linearPartOfMM * in_Tangent.xyz; //normalize(normalMatrix * in_Tangent);
	
	// Calculate matrices
	mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;

	// Multiply mvp matrix onto the vertex coordinates.
	gl_Position = mvp * vec4(modelSpacePosition, 1);
	// Multiply the model matrix onto the coordinate to get world-space coordinates.
	position = (modelMatrix * vec4(modelSpacePosition, 1)).xyz;
	
	/// Multiply with transpose of inverse modelView matrix
	// Normalize the normal.
	v_normal = normalize(mat3(normalMatrix) * in_Normal).xyz;

	
	// Just pass on the UV-coordinates ^^
	UV_Coord = in_UV;
	// Calculate vector to eye from vertex.
	vecToEye = normalize((eyePosition.xyz - position.xyz));
}

/*
#version 150




void main(void) {
	
	
}

*/