// Author: Emil Hedemalm
// Date: 2016-06-20
// Name: Deferred lighting n shadow mapping
#version 330

// Uniforms
// 2D Texture texture. Set these to -1 if you are not going to use them!
uniform sampler2D diffuseMap; // Same namespace as the regular diffuse/specular/normal maps in 
uniform sampler2D specularMap; // Simple/non-deferred or in deferred-gatherer render-passes.
uniform sampler2D normalMap;
uniform sampler2D positionMap; // For deferred.
uniform sampler2D emissiveMap;
uniform sampler2D depthMap; // For writing the correct depth from earlier.

// Lights
#define MAX_LIGHTS	150
uniform vec4 light_ambient = vec4(1,1,1,1);
uniform vec4 light_diffuse[MAX_LIGHTS];
uniform vec4 light_specular[MAX_LIGHTS];
uniform vec3 light_position[MAX_LIGHTS];	
uniform vec3 light_attenuation[MAX_LIGHTS];	// Constant, linear and quadratic
uniform int light_castsShadows[MAX_LIGHTS]; // -1 if not, positive if a specific shadow map? :D

// Position or direction, depending on w-parameter 0 = direction, 1 = positional
uniform int light_type[MAX_LIGHTS];	

// Spotlight data
uniform vec3 light_spotDirection[MAX_LIGHTS];
uniform float light_spotCutoff[MAX_LIGHTS];
uniform int light_spotExponent[MAX_LIGHTS];
// Total amount of active lights
uniform int activeLights = 0;	// Lights active, from index 0 to activeLights-1

/// Variables needing replacement/new buffers/textures/uniforms.
/*
	material-properties.
	emissiveMap
*/
uniform vec4	materialAmbient		= vec4(0.2, 0.2, 0.2, 1.0);
uniform vec4	materialDiffuse		= vec4(0.8, 0.8, 0.8, 1.0);
uniform vec4	materialSpecular	= vec4(1.1, 1.1, 1.1, 1.0);
uniform int		materialShininess	= 8	;

/// Shadow-mapping o.o
uniform sampler2D shadowMap;
uniform mat4 shadowMapMatrix;

// Matrix to be multiplied for the normals (for rotations primarily)
uniform mat4 normalMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// Position of eye in the world.
uniform vec4 eyePosition	= vec4(0.0, 5.0, 0.0, 0.0);

// Input data from the fragment shader
varying vec2 UV_Coord;		// Just passed on
varying vec3 constantColor;

/// Explicit out locations for the destination FrameBuffer 
// Use for later maybe, next pass.
// layout (location = 0) out vec4 diffuseOut;
uniform int pickingID = 1;
uniform vec4 primaryColorVec4 = vec4(1,1,1,1);

/// Gets attenuation based on vectors of constant, linear, quadratic + distance to object.
float GetAttenuation(vec3 att, float dist)
{
	return att.x / (1 + att.y * dist + att.z * pow(dist, 2)); 
}

void main(){
	// Texture image data. This will be the base for the colors.
	vec3 white = vec3(1,1,1);

	vec3 position = texture2D(positionMap, UV_Coord).xyz;
	vec3 vecToEye = position - eyePosition.xyz + vec3(0,5,0);
	/// Assume eye is at 0,0,0 for now
	float distance = length(vecToEye);
	float intensity = 1.0f;
	intensity = 1.0f / (1.0f + distance);
	
	vec3 diffuse = texture2D(diffuseMap, UV_Coord).xyz;
	vec3 specular = texture2D(specularMap, UV_Coord).xyz;
	vec3 normal = texture2D(normalMap, UV_Coord).xyz;
	// DO NOTE: Alpha has to be Positive for the value to be printed out to the texture AT ALL! 
	// It defines probably the ratio at which it is sent down the pipeline. o-o;;
	/*
	normalOut = vec4((normalize(normal)).xyz, 1);
	if (true)
		diffuseOut = texture2D(diffuseMap, UV_Coord) * primaryColorVec4;
	else
		diffuseOut = vec4(1.3,0.3,0.3,1.0) * primaryColorVec4;
*/	
	vec3 color = vec3(0,0,0);
//	color = specular;
//	color = position;
//	color.xy += UV_Coord * 0.2;

	// Luminosity for each color
	vec3 diffuseLuminosity = vec3(0,0,0);
	vec3 specularLuminosity = vec3(0,0,0);

	bool doLights = true;
	if (doLights)
	{
		for (int i = 0; i < activeLights; ++i)
		{			
			vec3 lightLuminosity = vec3(0,0,0);
			vec3 lightSpecular = vec3(0,0,0);
					
	//		lightLuminosity.xyz = vec3(1,1,1);
	//		lightSpecular.xyz = vec3(1,1,1);
			// Directional lighting ^^
			if (light_type[i] == 2){
			
				// Diffuse
				vec3 lightDirection = normalize(light_position[i].xyz);
				vec3 normalizedNormal = normalize(vec3(normal.xyz));
				float difIntensity = max(dot(lightDirection, normalizedNormal), 0.0);
				/// Opt out if the diffuse-intensity is 0. (L-Dot-N == 0) 
				/// The specular has no chance to be above 0 if this is the case!
				if (difIntensity <= 0)
					continue;
				lightLuminosity += difIntensity * light_diffuse[i].xyz; // * materialDiffuse;
			
				// Only appl7y constant light attenuation for the directional lights!?
				float distance = length(light_position[i].xyz - position.xyz);
				float attenuation = GetAttenuation(light_attenuation[i], distance);
				lightLuminosity *= attenuation;

				// Specular - intensity same, but consider the half vector and eye position.
				vec3 vectorToEye = normalize(vecToEye);
				vec3 halfVector = normalize((lightDirection + vectorToEye));
				float initialBrightness = max(dot(halfVector, normalizedNormal), 0.0);
				
				int smoothness = 12;
				float totalBrightness = initialBrightness;
				for (int i = 0; i < materialShininess; ++i){
					totalBrightness *= initialBrightness;
				}
				lightSpecular += totalBrightness * light_specular[i].xyz;
				lightSpecular *= attenuation;
				
			//	lightSpecular *= 0.1;
			//		lightSpecular *= 0;
			
			}
			
			// Positional lighting o-o;
			else if (light_type[i] == 1){
			//	continue;
				 
				// Diffuse
				vec3 lightDirection = normalize(position.xyz - light_position[i].xyz);
				vec3 normalizedNormal = normalize(vec3(normal.xyz));
				
				float difIntensity = max(-dot(lightDirection, normalizedNormal), 0.0);
		
	//	constantColor = normalizedNormal;
	//			constantColor.x = difIntensity;
	//
				/// Opt out if the diffuse-intensity is 0. (L-Dot-N == 0) 
				/// The specular has no chance to be above 0 if this is the case!
				if (difIntensity <= 0)
					continue;
							
				lightLuminosity += difIntensity * light_diffuse[i].xyz; // * materialDiffuse;
				
			//	gl_FragColor.xyz += lightLuminosity * texel.xyz;


				// Only apply constant light attenuation for the directional lights!?
				float distance = length(light_position[i].xyz - position.xyz);
				float attenuation = GetAttenuation(light_attenuation[i], distance);
				lightLuminosity *= attenuation;
				
				// Specular - intensity same, but consider the half vector and eye position.
				// First calc vector to eye from the fragment
				vec3 vectorToEye = normalize(vecToEye);
			
				// Calculate the mid-way vector between the angle of the light and the vector to the eye.
				// Since the light direction is in the opposite direction at first, negate it.
				vec3 halfVector = normalize((-lightDirection + vectorToEye) / 2);
				float initialBrightness = max(dot(halfVector, normalizedNormal), 0.0);

				//	constantColor.x = initialBrightness;


				
				int smoothness = 12;
				float totalBrightness = initialBrightness;
				for (int i = 0; i < materialShininess; ++i){
					totalBrightness *= initialBrightness;
				}
				lightSpecular += totalBrightness * light_specular[i].xyz;
				lightSpecular *= attenuation;
		
				lightSpecular *= 0.3;
		
			}
			// Spotlights!!!
			else if (light_type[i] == 3){
			//continue;
				/// First compare lightDirection with the spot's direction.
				vec3 lightDirection = normalize(light_position[i].xyz - position.xyz);
				// Check if we should do more calculations at all or not...
				float spotDotCutoff = dot(normalize(-light_spotDirection[i]), lightDirection);
				if (spotDotCutoff > light_spotCutoff[i]){
					// Now just do normal calculations as per positional light-sources ^^
					vec3 normalizedNormal = normalize(vec3(normal.xyz));
					float difIntensity = max(dot(lightDirection, normalizedNormal), 0.0);
					lightLuminosity += difIntensity * light_diffuse[i].xyz; // * materialDiffuse;
					// Only apply constant light attenuation for the directional lights!?
					float distance = length(light_position[i].xyz - position.xyz);
					float attenuation = GetAttenuation(light_attenuation[i], distance);
					lightLuminosity *= attenuation;
			
					// Specular - intensity same, but consider the half vector and eye position.
					vec3 vectorToEye = normalize(vecToEye);
					vec3 halfVector = normalize((lightDirection + vectorToEye) / 2);
					float initialBrightness = max(dot(halfVector, normalizedNormal), 0.0);
					int smoothness = 12;
					float totalBrightness = initialBrightness;
					for (int i = 0; i < materialShininess; ++i){
						totalBrightness *= initialBrightness;
					}
					lightSpecular += totalBrightness * light_specular[i].xyz;
					lightSpecular *= attenuation;

					// Calculate the spotlight exponent
					float spotIntensity = 1.0f;
					for (int j = 0; j < light_spotExponent[i]; ++j){
						spotIntensity *= spotDotCutoff;
					}
					/// Apply the spot intensity to both the added specular and diffuse luminosity
					lightSpecular *= spotIntensity;
					lightLuminosity *= spotIntensity;
				}
			//	lightLuminosity *= 0;
			}

			// o.o;
			if (light_castsShadows[i] >= 0)
			{
				// replace diffuse-map with the shadow-map?
				int shadowSamples = 9;
				/// Should use like, 1 sample, 3, 5, or 9, 13, etc.
				float inShadow = 0;
				float ssd = 0.00025; /// shadow sampler distance
				float ssd2 = ssd*0.5;
				for (int i = 0; i < shadowSamples; ++i)
				{
					// Check stuff.
					vec3 shadowPos = position;
					vec4 shadowSpace = shadowMapMatrix * vec4(shadowPos, 1);
					if (i == 1) 		shadowSpace.xy += vec2(ssd,ssd);  // 
					else if (i == 2)	shadowSpace.xy += vec2(-ssd,-ssd); //-
					else if (i == 3)	shadowSpace.xy += vec2(ssd,-ssd);
					else if (i == 4)	shadowSpace.xy += vec2(-ssd,ssd);
					else if (i == 5)	shadowSpace.xy += vec2(-ssd,0); // +
					else if (i == 6)	shadowSpace.xy += vec2(ssd,0);
					else if (i == 7)	shadowSpace.xy += vec2(0,-ssd);
					else if (i == 8)	shadowSpace.xy += vec2(0,ssd);
					else if (i == 9)	shadowSpace.xy += vec2(ssd2,ssd); // / & \ ?
					else if (i == 10)	shadowSpace.xy += vec2(-ssd2,-ssd); // / & \ ?
					else if (i == 11)	shadowSpace.xy += vec2(-ssd2,ssd); // / & \ ?
					else if (i == 12)	shadowSpace.xy += vec2(ssd2,-ssd); // / & \ ?
					else if (i == 13)	shadowSpace.xy += vec2(ssd,ssd2); // Like 9 but horizontal
					else if (i == 14)	shadowSpace.xy += vec2(-ssd,-ssd2); // / & \ ?
					else if (i == 15)	shadowSpace.xy += vec2(ssd,-ssd2); // / & \ ?
					else if (i == 16)	shadowSpace.xy += vec2(-ssd,ssd2); // / & \ ?
					
					float depthAtLocationInShadowMap = texture2D(shadowMap, shadowSpace.xy).z;
					float fragmentDepthInLightSpace = shadowSpace.z;
					fragmentDepthInLightSpace = fragmentDepthInLightSpace;
					if (depthAtLocationInShadowMap < fragmentDepthInLightSpace - 0.001)
						inShadow += 1;
				// IF false positive, due to co-ordinates outside the map, make it shadow? If not including all objects, instead extend the shadow map/camera properties of the light rendering setup?
				if (shadowSpace.x > 1 || shadowSpace.x < 0 ||
					shadowSpace.y > 1 || shadowSpace.y < 0 ||
					shadowSpace.z > 1 || shadowSpace.z < 0)
					inShadow += 2;

			//		constantColor.xyz = vec3(1,1,1) * depthAtLocationInShadowMap;
			//		constantColor.xyz = vec3(1,1,1) * fragmentDepthInLightSpace;
			//			constantColor.x = shadowSpace.x;					
				}
				inShadow /= shadowSamples;
				lightSpecular.xyz *= 1-inShadow;
				lightLuminosity.xyz *= 1-inShadow;
			}

			
			// Add the diffuseLuminosity
			diffuseLuminosity += lightLuminosity;
			specularLuminosity += lightSpecular;
		}	
	}

	// Final illumination done, multiply with diffuse ^^
	vec3 diffuseTotal = diffuse.xyz * diffuseLuminosity * materialDiffuse.xyz;
	vec3 specularTotal = vec3(0,0,0);
	/// Check if using specular map.
	vec4 specTex = texture2D(specularMap, UV_Coord);
	specularTotal.xyz += specTex.xyz * specularLuminosity * materialSpecular.xyz;
	
	// Sample emissive map?
	vec3 emissive = texture2D(emissiveMap, UV_Coord).xyz;
	
	/// Add global ambient ^^
	vec3 diffuseFactor = clamp(diffuseTotal.xyz, 0, 1);
	vec3 specularFactor = clamp(specularTotal, 0, 1);
	vec3 emissiveFactor = clamp(emissive, 0, 1) * 0;
	vec3 ambientFactor = clamp(diffuse.xyz * light_ambient.xyz, 0, 1);
	
	color = diffuseFactor + specularFactor + emissiveFactor + ambientFactor;

	if (constantColor.x != 0)
		color = constantColor;
//	color.x += distance;
//	color = specTex.xyz;
//	color = diffuse.xyz;
		
	gl_FragColor.xyz = color;
	gl_FragColor.w = distance;
	gl_FragDepth = texture2D(depthMap, UV_Coord).x;
//	gl_Position.w = distance;
	return;
}
