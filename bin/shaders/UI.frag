// Author: Emil Hedemalm
// Date: 2012-10-29
// Name: Simple UI Shader
#version 120

// Uniforms
// 2D Texture texture
uniform sampler2D baseImage;

// Light statistics
uniform bool	lightActive				= true;
uniform vec4	lightAmbient			= vec4(0.0, 0.0, 0.0, 1.0);
uniform vec4	lightDiffuse			= vec4(1.0, 1.0, 1.0, 1.0);
uniform vec4	lightSpecular			= vec4(1.0, 1.0, 1.0, 1.0);
uniform vec4	lightDirOrPos			= vec4(1.0, 2.0, -1.0, 1.0);
uniform int		lightType				= 0;
uniform vec3	lightAttenuation		= vec3(1.0, 0.0, 0.0);

// Material statistics
uniform vec4	materialAmbient		= vec4(0.2, 0.2, 0.2, 1.0);
uniform vec4	materialDiffuse		= vec4(0.8, 0.8, 0.8, 1.0);
uniform vec4	materialSpecular	= vec4(1.1, 1.1, 1.1, 1.0);
uniform int		materialShininess	= 8;

// Yush.
uniform vec4	primaryColorVec4 = vec4(1,1,1,1);
/// Highlight that is added linearly upon the final product.
uniform vec4	highlightColorVec4 = vec4(0,0,0,0);

// Input data from the fragment shader
varying vec3 normal;		// Interpollated coordinates that have been transformed to view space
varying vec2 UV_Coord;	// Just passed on
varying vec3 worldCoord;	// World coordinates of the fragment
varying vec3 vecToEye;	// Vector from vertex to eye
varying vec3 position;


	
void main(void) 
{
	// Texture image data. This will be the base for the colors.
	vec4 baseFrag = texture2D(baseImage, UV_Coord);
	vec4 color = baseFrag;
/*	
	color *= primaryColorVec4;
	color += highlightColorVec4;
	
	gl_FragColor = color;
	*/
//	float highlight = primaryColorVec4.x - 1.0f;
//	if (highlight >= 0)
//		baseFrag.xyz += vec3(1.0f,1.0f,1.0f) * highlight;

	gl_FragColor = baseFrag;
	gl_FragColor += vec4(position.x / 20000, position.y / 12000, position.z / 40, 0)*1.0;
	gl_FragColor *= primaryColorVec4;
	gl_FragColor += highlightColorVec4;
	
//	gl_FragColor.x = 1;
//	gl_FragColor = vec4(0,0,0,1);
//	return;
	
}


