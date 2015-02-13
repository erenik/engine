// Author: Emil Hedemalm
// Date: 2012-10-29
#version 120

// For bit-wise operations!
// #extension GL_EXT_gpu_shader4 : enable

uniform float fogBegin = 500.0;
uniform float fogEnd = 2500.0;
uniform vec3 fogColor = vec3(0,0,0);

// Input data from the fragment shader
varying vec3 position;	// World coordinates of the fragment

void main()
{
	// Set depth first..
	float depth = gl_FragCoord.z;
	/// Set depth of the pixel as it was!
	gl_FragDepth = depth; //depth.x;
	gl_FragColor.xyzw = vec4(1,1,1,1);	
}

