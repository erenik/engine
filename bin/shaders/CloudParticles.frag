// Author: Emil Hedemalm
// Date: 2014-09-17
#version 120

// Uniforms
// 2D Texture texture
uniform sampler2D baseImage;

// Color applied to all stuff in the final.
uniform vec4 primaryColorVec4 = vec4(1.0, 1.0, 1.0, 1.0);

// Light data
uniform vec4 global_ambient = vec4(1,1,1,1);

// Input data from the fragment shader
varying vec3 v_w_normal;		// Interpolated coordinates that have been transformed to view space
varying vec2 v_UV_Coord;	// Just passed on
varying vec3 worldCoord;	// World coordinates of the fragment
varying vec3 vecToEye;	// Vector from vertex to eye
varying vec3 v_w_position;

// Particle color
varying vec4 color;
varying vec2 lifeTimeDuration;
varying vec3 debugColor;

/// 0 - No, 1 - linearly, 2 - quadratically, 3 - cubically
uniform int particleDecayAlphaWithLifeTime;

void main(){
	// Texture image data. This will be the base for the colors.
	vec4 baseFrag = texture2D(baseImage, v_UV_Coord);

	baseFrag *= color;
	float lifeTime = lifeTimeDuration.x;
	float halfLife = lifeTime * 0.5;
	float duration = lifeTimeDuration.y;
	float relativeTimeLived = clamp(duration / lifeTime, 0, 1);
	// None
	if(particleDecayAlphaWithLifeTime == 0)
		;
	// Linear
	else if(particleDecayAlphaWithLifeTime == 1)
		baseFrag.w *= 1 - relativeTimeLived;
	// Cubic
	else if(particleDecayAlphaWithLifeTime == 2)
		baseFrag.w *= 1 - pow(duration / lifeTime, 2);
	// Cubic
	else if(particleDecayAlphaWithLifeTime == 3)
		baseFrag.w *= 1 - pow(duration / lifeTime, 3);
	// Lighting?
	
	baseFrag = color;
	
	// Vary alpha with life-time/duration
	if (relativeTimeLived < 0.3)
		baseFrag.w *= (relativeTimeLived / 0.3f);
	else if (relativeTimeLived > 0.7)
	{
		//                 0 to 0.3   ->   0 to 1.0
		baseFrag.w *=  1 - (relativeTimeLived - 0.7) / 0.3;
	}
//	baseFrag.w = relativeTimeLived;
//	v_w_position
	
	// Highlights from the sun!
	if (dot(v_w_normal, vec3(1,1,0)) > 0.9)
		baseFrag.xyz += 0.1;
		
	// Darkness from the rain! o.o
	if (dot(v_w_normal, vec3(0,-1,0)) > 0.5)
	{
		baseFrag.xyz -= 0.1;
	}
	
	gl_FragColor = baseFrag;
	if (debugColor.x > 0 || debugColor.y > 0 || debugColor.z > 0)
		gl_FragColor.xyz = debugColor;
}
