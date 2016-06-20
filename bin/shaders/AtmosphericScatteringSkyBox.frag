// Author: Emil Hedemalm
// Date: 2012-10-29
#version 120

// For bit-wise operations!
// #extension GL_EXT_gpu_shader4 : enable

uniform float fogBegin = 500.0;
uniform float fogEnd = 2500.0;
uniform vec3 fogColor = vec3(0,0,0);

uniform vec4 ambientVec4 = vec4(0.5,0.5,0.5,1.0);

uniform vec3 sunPosition = vec3(0,0,-1);
uniform vec4 sunColor = vec4(1,1,1,1);

uniform vec3 skyColor = vec3(0.78,0.89,1);

// Input data from the fragment shader
varying vec3 vertexPosition;	// World coordinates of the fragment

void main()
{

	/// Just pass along the world-position... or possibly normalize it?
	vec3 position = normalize(vertexPosition.xyz);

	// Set depth first..
	float depth = gl_FragCoord.z;
	/// Set depth of the pixel as it was!
	gl_FragDepth = depth; //depth.x;
	
	vec3 color;
	color = vec3(1,1,1) * 0.5;
//	color.xyz += position;


	// Above the horizon.
	bool aboveTheHorizon = false;
	if (position.y > 0)
	{
		aboveTheHorizon = true;
		color = skyColor;
		/// Darken colors close to the bottom edge? 
		float edge = 0.3;
		if (position.y < edge)
		{
			float multiplier = 0.3;
			color *= (position.y / (edge / multiplier)) + ( 1 -multiplier);
		}
	}
	// Below the horizon, just some test-colors.
	if (!aboveTheHorizon)
	{
		vec3 normalizedPosition = position;
		if (dot(normalizedPosition, vec3(1,0,0)) > 0.9)
			color.x += 0.5;
		if (dot(normalizedPosition, vec3(0,0,1)) > 0.9)
			color.z += 0.5;
		if (dot(normalizedPosition, vec3(0,-1,0)) > 0.9)
			color.y += 0.5;
	}
	
	// check da sun!
	float dotSun = dot(sunPosition, position);
	if (dotSun > 0)
	{
		// Actual sun here, yo.
		float distToSun = length(position - sunPosition);
		if (distToSun < 0.025)
			color += sunColor.xyz * 1;
	;		
		// Irradience below?
		// Bright spot close to it.
		color += sunColor.xyz * pow(dotSun,120) * 0.2;
		// Illuminate much of the sky around it.
		color += sunColor.xyz * pow(dotSun,4) * 0.12;
		// If sunset, make the rest of the sky more its color?
		if (sunPosition.y < 0.6)
		{
			color += sunColor.xyz * pow(dotSun,1) * 0.1 * (0.4 - sunPosition.y);
		}
	}
		
	gl_FragColor.xyz = color;
	gl_FragColor.w = 1;
	
}

