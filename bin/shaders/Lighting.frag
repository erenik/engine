// Author: Emil Hedemalm
// Date: 2012-10-29
#version 120


// For bit-wise operations!
#extension GL_EXT_gpu_shader4 : enable

// Uniforms
uniform vec4 eyePosition	= vec4(0.0, 5.0, 0.0, 0.0);	// Position of eye in the world.

// Lights
#define MAX_LIGHTS	20
uniform vec3 global_ambient = vec3(1,1,1);
uniform vec4 light_diffuse[MAX_LIGHTS];
uniform vec4 light_specular[MAX_LIGHTS];
uniform vec3 light_position[MAX_LIGHTS];	// Position or direction, depending on w-parameter 0 = direction, 1 = positional
uniform vec3 light_attenuation[MAX_LIGHTS];	// Constant, linear and quadratic
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
uniform int		materialShininess	= 8;


// For if-checking for applying normal maps, specular maps, etc.
#define DIFFUSE_MAP		0x0000001
#define SPECULAR_MAP	0x0000002
#define NORMAL_MAP		0x0000004
uniform int texturesToApply = 0; // DIFFUSE_MAP | SPECULAR_MAP | NORMAL_MAP;

// 2D Textures
uniform sampler2D diffuseMap;
uniform sampler2D depthMap;
uniform sampler2D normalMap;
uniform sampler2D positionMap;
uniform sampler2D specularMap; // Wosh!
uniform sampler2D tangentMap;	// For normal-mapping?
uniform sampler2D bumpMap; // For-- normal-mapping.. wanted to name it normalMap, but eh..
uniform sampler2D pickingMap;	// For visualizing picking

// For adjusting colors globally.
uniform vec4	primaryColorVec4	= vec4(1.0, 1.0, 1.0, 1.0);

// Input data from the fragment shader
in vec2 UV_Coord;	// Just passed on

// Out color

void main(){
	// Texture image data. This will be the base for the colors.
	vec4 diffuseTexel = texture2D(diffuseMap, UV_Coord);
	vec4 depthTexel = texture2D(depthMap, UV_Coord);
	vec4 v_normal = texture2D(normalMap, UV_Coord);
	vec4 position = texture2D(positionMap, UV_Coord);
	vec4 v_Tangent = texture2D(tangentMap, UV_Coord);
	vec4 specular = texture2D(specularMap, UV_Coord);
	vec4 picking = texture2D(pickingMap, UV_Coord);
	vec4 normalMapTexel = texture2D(bumpMap, UV_Coord);
	
	/// Call the blended texture fragment Texel ^^ Could be derived from multiple textures! ^.^
	vec4 texel = diffuseTexel;	
	float depth = depthTexel.x;

	/// Set depth of the pixel as it was!
	gl_FragDepth = depth; //depth.x;
	
	vec3 normal = v_normal.xyz;
	vec3 tangent = v_Tangent.xyz;

	// Blue value at 100% = no bump-mapping needed on this pixel
	if (false   
		&& normalMapTexel.z < 1.0
		
	/*  && 	normalMapTexel.z  != 0  */
		){
		vec4 nmTexel = normalMapTexel;
	//	texel.xyz = nmTexel.xyz * 0.05 + texel.xyz * 1;
		// Do magic!
		tangent = normalize(tangent);
		vec3 biTangent = cross(tangent, normal);
		biTangent = normalize(biTangent);
		if (v_Tangent.w >= 0.9){
		//	texel += 0.0;
		//	tangent.y *= -1;
		//	biTangent.y *= -1;
		//	gl_FragColor = vec4(1,1,1,1);
		//		return;
		}
	//	texel.xyz = biTangent;
	//	texel.xyz = normal;
		
		normal = nmTexel.z * normal;
		normal += nmTexel.x * tangent.xyz;
		normal += nmTexel.y * biTangent.xyz;
		normal = normalize(normal);
		
		gl_FragColor.xyz = normalMapTexel.xyz;
		gl_FragColor.w = 1;
	//	return;
	//	+ nmTexel.y * biTangent;
	//	normal.xyz = normalMapTexel.xyz - 0.5;
	//	normal = normalize(normal);
	}		
	
	
	/// Background static ambience if depth is 1 (farplane)
	if (depth == 1){
		gl_FragColor = vec4(global_ambient, 1);
		return;
	}
	
	// Calculate vector to eye from vertex.
	vec3 vecToEye = eyePosition.xyz - position.xyz;
	gl_FragColor.xyz += vecToEye;
	gl_FragColor.xyz += eyePosition.xyz;
	/// Set color to 0.
	gl_FragColor = vec4(0,0,0,0);
	/// Set alpha now.
	gl_FragColor.w += texel.w;

	if (normal.x < 0.1 && normal.y < 0.1 && normal.z < 0.1)
	;//	normal.xyz = vec3(0,1,0);

	// Luminosity for each color
	vec3 diffuseLuminosity = vec3(0,0,0);
	vec3 specularLuminosity = vec3(0,0,0);

	bool doLights = true;
	if (doLights)
	for (int i = 0; i < activeLights; ++i){
			
		vec3 lightLuminosity = vec3(0,0,0);
		vec3 lightSpecular = vec3(0,0,0);
		// Directional lighting ^^
		if (light_type[i] == 2){
		
			// Diffuse
			vec3 lightDirection = normalize(light_position[i].xyz);
			vec3 normalizedNormal = normalize(vec3(normal.xyz));
			float difIntensity = max(dot(lightDirection, normalizedNormal), 0);
			/// Opt out if the diffuse-intensity is 0. (L-Dot-N == 0) 
			/// The specular has no chance to be above 0 if this is the case!
			if (difIntensity <= 0)
				continue;
			lightLuminosity += difIntensity * light_diffuse[i].xyz; // * materialDiffuse;
		
			// Only apply constant light attenuation for the directional lights!?
			float distance = length(light_position[i].xyz - position.xyz);
			float attenuation = 1 / (light_attenuation[i].x + light_attenuation[i].y * distance + light_attenuation[i].z * pow(distance, 2)); 
			lightLuminosity *= attenuation;

			// Specular - intensity same, but consider the half vector and eye position.
			vec3 vectorToEye = normalize(vecToEye);
			vec3 halfVector = normalize((lightDirection + vectorToEye));
			float initialBrightness = max(dot(halfVector, normalizedNormal), 0);
			int smoothness = 12;
			float totalBrightness = initialBrightness;
			for (int i = 0; i < materialShininess; ++i){
				totalBrightness *= initialBrightness;
			}
			lightSpecular += totalBrightness * light_specular[i].xyz;
			lightSpecular *= attenuation;
		//		lightSpecular *= 0;
		
		}
		
		// Positional lighting o-o;
		else if (light_type[i] == 1){
		//	continue;
			// Diffuse
			vec3 lightDirection = normalize(light_position[i].xyz - position.xyz);
			vec3 normalizedNormal = normalize(vec3(normal.xyz));
			float difIntensity = max(dot(lightDirection, normalizedNormal), 0);
			/// Opt out if the diffuse-intensity is 0. (L-Dot-N == 0) 
			/// The specular has no chance to be above 0 if this is the case!
			if (difIntensity == 0)
				continue;
			
			lightLuminosity += difIntensity * light_diffuse[i].xyz; // * materialDiffuse;
			
			// Only apply constant light attenuation for the directional lights!?
			float distance = length(light_position[i].xyz - position.xyz);
			float attenuation = 1 / (light_attenuation[i].x + light_attenuation[i].y * distance + light_attenuation[i].z * pow(distance, 2)); 
			lightLuminosity *= attenuation;
			
			// Specular - intensity same, but consider the half vector and eye position.
			vec3 vectorToEye = normalize(vecToEye);
			vec3 halfVector = normalize((lightDirection + vectorToEye) / 2);
			float initialBrightness = max(dot(halfVector, normalizedNormal), 0);
			int smoothness = 12;
			float totalBrightness = initialBrightness;
			for (int i = 0; i < materialShininess; ++i){
				totalBrightness *= initialBrightness;
			}
			lightSpecular += totalBrightness * light_specular[i].xyz;
			lightSpecular *= attenuation;
		}
		// Spotlights!!!
		else if (light_type[i] == 3){
			/// First compare lightDirection with the spot's direction.
			vec3 lightDirection = normalize(light_position[i].xyz - position.xyz);
			// Check if we should do more calculations at all or not...
			float spotDotCutoff = dot(normalize(-light_spotDirection[i]), lightDirection);
			if (spotDotCutoff > light_spotCutoff[i]){
				// Now just do normal calculations as per positional light-sources ^^
				vec3 normalizedNormal = normalize(vec3(normal.xyz));
				float difIntensity = max(dot(lightDirection, normalizedNormal), 0);
				lightLuminosity += difIntensity * light_diffuse[i].xyz; // * materialDiffuse;
				// Only apply constant light attenuation for the directional lights!?
				float distance = length(light_position[i].xyz - position.xyz);
				float attenuation = 1 / (light_attenuation[i].x + light_attenuation[i].y * distance + light_attenuation[i].z * pow(distance, 2)); 
				lightLuminosity *= attenuation;
		
				// Specular - intensity same, but consider the half vector and eye position.
				vec3 vectorToEye = normalize(vecToEye);
				vec3 halfVector = normalize((lightDirection + vectorToEye) / 2);
				float initialBrightness = max(dot(halfVector, normalizedNormal), 0);
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
		}
		// Add the diffuseLuminosity
		diffuseLuminosity += lightLuminosity;
		specularLuminosity += lightSpecular;
	}	
	
	
	// Final illumination done, multiply with diffuse ^^
	gl_FragColor.xyz += texel.xyz * diffuseLuminosity * materialDiffuse.xyz;
	
	/// Check if using specular map.
	if (true){
		vec4 specTex = texture2D(specularMap, UV_Coord);
		gl_FragColor.xyz += specTex.xyz * specularLuminosity * materialSpecular.xyz;
	}
	// Default, use diffuse-map as specular.
	else
		gl_FragColor.xyz += texel.xyz * specularLuminosity * materialSpecular.xyz;
	
	/// Add global ambient ^^
	gl_FragColor.xyz += texel.xyz * global_ambient * materialAmbient.xyz;
	
	gl_FragColor.xyz *= primaryColorVec4.xyz;
	
	// Debug-rendering
	gl_FragColor.w += 2;
	
	// First make sure all are in so that it compiles..
	gl_FragColor.xyz += diffuseTexel.xyz * 0.01;
	gl_FragColor.xyz += position.xyz * 0.000005;
	gl_FragColor.xyz += normal.xyz * 0.005;
	gl_FragColor.xyz += specular.xyz * 0.01;
	// All above are tested and work!
	
	gl_FragColor.xyz = clamp(gl_FragColor.xyz, vec3(0,0,0), vec3(1,1,1));
	gl_FragColor.xyz += max(tangent.xyz,vec3(0,0,0)) * 0.01;
	gl_FragColor.xyz += normalMapTexel.xyz * 0.01;
	gl_FragColor.xyz += picking.x * 0.01;
	
	
//	gl_FragColor.xyz += normalMapTexel.xyz * 0.5;
//	gl_FragColor.xyz += specular.xyz * 5;
}
