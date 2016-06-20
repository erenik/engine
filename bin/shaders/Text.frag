// Author: Emil Hedemalm
// Date: 2012-10-29
// Name: Simple UI Shader
#version 120

// Uniforms
// 2D Texture texture
uniform sampler2D baseImage;

// Yush.
uniform vec4	primaryColorVec4 = vec4(1,1,1,1);
/// Highlight that is added linearly upon the final product.
uniform vec4	highlightColorVec4 = vec4(0,0,0,0);

// Input data from the fragment shader
varying vec2 UV_Coord;	// Just passed on
varying vec3 worldCoord;	// World coordinates of the fragment
varying vec3 vecToEye;	// Vector from vertex to eye
varying vec3 position;

void main(void) 
{
	// Texture image data. This will be the base for the colors.
	vec4 baseFrag = texture2D(baseImage, UV_Coord);
	vec4 color = baseFrag;

	
	gl_FragColor = baseFrag;
	gl_FragColor *= primaryColorVec4;
	gl_FragColor += highlightColorVec4;	
//	gl_FragColor.xw = vec2(.5,0);
}


