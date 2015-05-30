// Author: Emil Hedemalm
// Date: 2012-10-29
#version 120

// For bit-wise operations!
// #extension GL_EXT_gpu_shader4 : enable

// Uniforms
// Position of eye in the world.
uniform vec4 eyePosition	= vec4(0.0, 5.0, 0.0, 0.0);	
// Color applied to all stuff in the final.
uniform vec4 primaryColorVec4 = vec4(1.0, 1.0, 1.0, 1.0);

// Lights
#define MAX_LIGHTS	8
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

// Material statistics
uniform vec4	materialAmbient		= vec4(0.2, 0.2, 0.2, 1.0);
uniform vec4	materialDiffuse		= vec4(0.8, 0.8, 0.8, 1.0);
uniform vec4	materialSpecular	= vec4(1.1, 1.1, 1.1, 1.0);
uniform int		materialShininess	= 8	;

// 2D Texture texture
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;
uniform sampler2D emissiveMap;

uniform float emissiveMapFactor = 1.0;

/// Shadow-mapping o.o
uniform sampler2D shadowMap;
uniform mat4 shadowMapMatrix;

/// Individual bools for if the above samplers are active or not!
uniform bool useDiffuseMap;
uniform bool useSpecularMap;
uniform bool useNormalMap;

uniform float fogBegin = 500.0;
uniform float fogEnd = 2500.0;
uniform vec3 fogColor = vec3(0,0,0);

// Input data from the fragment shader
varying vec3 v_normal;		// Interpolated coordinates that have been transformed to view space
varying vec2 UV_Coord;	// Just passed on
varying vec3 position;	// World coordinates of the fragment
varying vec3 vecToEye;	// Vector from vertex to eye
varying vec4 v_Tangent;	// Tangent XY in XY and CoTangent XY in ZW?

void main()
{
	// For testing shit
	vec3 constantColor = vec3(0,0,0);

//	constantColor.x = 1;

	// Set depth first..
	float depth = gl_FragCoord.z;
	/// Set depth of the pixel as it was!
	gl_FragDepth = depth; //depth.x;
	// Base texel for color, usually gathered from the DiffuseMap.
	vec4 texel;
	// Texture image data. This will be the base for the colors.
	if (useDiffuseMap){
		/// Call the blended texture fragment Texel ^^ Could be derived from multiple textures! ^.^
		 texel = texture2D(diffuseMap, UV_Coord);	
	}
	else 
		texel = vec4(0.3,0.3,0.3,1.0);
	
	gl_FragColor = texel;
	/// Background static ambience if depth is 1 (farplane)
	if (depth == 1){
//		gl_FragColor = light_ambient;
//		return;
	}

	
	
	vec3 normal = v_normal;

//	constantColor.xyz = normal.xyz;
//	constantColor.x += 0.1;

	
	// Calculate vector to eye from vertex.
	vec3 vecToEye = eyePosition.xyz - position.xyz;
	gl_FragColor.xyz += vecToEye;
	gl_FragColor.xyz += eyePosition.xyz;
	/// Set color to 0.
	gl_FragColor = vec4(0,0,0,0);
	/// Set alpha now.
	gl_FragColor.w += texel.w;

	
	
//	constantColor = normal;
	
//	constantColor.xyz = position.xyz;
//	

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
				float attenuation = 1 / (light_attenuation[i].x + light_attenuation[i].y * distance + light_attenuation[i].z * pow(distance, 2)); 
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
				float attenuation = 1 / (light_attenuation[i].x + light_attenuation[i].y * distance + light_attenuation[i].z * pow(distance, 2)); 
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
					float attenuation = 1 / (light_attenuation[i].x + light_attenuation[i].y * distance + light_attenuation[i].z * pow(distance, 2)); 
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
				// Check stuff.
				vec3 shadowPos = position;
				vec4 shadowSpace = shadowMapMatrix * vec4(shadowPos, 1);
				float depthAtLocationInShadowMap = texture2D(shadowMap, shadowSpace.xy).z;
				float fragmentDepthInLightSpace = shadowSpace.z;
				fragmentDepthInLightSpace = fragmentDepthInLightSpace;
				

		//		constantColor.xyz = vec3(1,1,1) * depthAtLocationInShadowMap;
		//		constantColor.xyz = vec3(1,1,1) * fragmentDepthInLightSpace;
		//			constantColor.x = shadowSpace.x;
			
				bool inShadow = false;
				if (depthAtLocationInShadowMap < fragmentDepthInLightSpace 
				- 0.0001
				)
					inShadow = true;
				// IF false positive, due to co-ordinates outside the map, make it shadow.
				if (shadowSpace.x > 1 || shadowSpace.x < 0 ||
					shadowSpace.y > 1 || shadowSpace.y < 0 ||
					shadowSpace.z > 1 || shadowSpace.z < 0)
					inShadow = true;
				if (inShadow)
				{
					lightSpecular.xyz *= vec3(0,0,0);
					lightLuminosity.xyz *= vec3(0,0,0);;
				}
			}

			
			// Add the diffuseLuminosity
			diffuseLuminosity += lightLuminosity;
			specularLuminosity += lightSpecular;
		}	
	}

	// Final illumination done, multiply with diffuse ^^
	vec3 diffuseTotal = texel.xyz * diffuseLuminosity * materialDiffuse.xyz;
	vec3 specularTotal = vec3(0,0,0);
	/// Check if using specular map.
	if (useSpecularMap)
	{
		vec4 specTex = texture2D(specularMap, UV_Coord);
		specularTotal.xyz += specTex.xyz * specularLuminosity * materialSpecular.xyz;
	}
	// Default, use diffuse-map as specular.
	else
		specularTotal.xyz += texel.xyz * specularLuminosity * materialSpecular.xyz;
	
	// Sample emissive map?
	vec3 emissive = texture2D(emissiveMap, UV_Coord).xyz * emissiveMapFactor;

	/// Add global ambient ^^
	vec3 diffuseFactor = clamp(diffuseTotal.xyz, 0, 1);
	vec3 specularFactor = clamp(specularTotal, 0, 1);
	vec3 emissiveFactor = clamp(emissive, 0, 1);
	vec3 ambientFactor = clamp(texel.xyz * light_ambient.xyz, 0, 1);
	
	gl_FragColor.xyz = diffuseFactor + specularFactor + emissiveFactor + ambientFactor;
	
	// Additional multiplier.
	gl_FragColor *= clamp(primaryColorVec4, 0.01, 1.0); 

//	constantColor.xyz = light_ambient.xyz;
	
	if (gl_FragColor.x < 0)
	{
//		constantColor.x = 0.1;
	}
	/// Debugging color.
	if (constantColor.x > 0 || constantColor.y > 0 || constantColor.z  > 0)
		gl_FragColor.xyz = constantColor;
}

