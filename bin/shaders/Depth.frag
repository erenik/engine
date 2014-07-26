// Author: Emil Hedemalm
// Date: 2012-10-29
#version 120

// Uniforms
// 2D Texture texture
uniform sampler2D baseImage;

// Input data from the fragment shader
in vec3 normal;		// Interpollated coordinates that have been transformed to view space
in vec2 UV_Coord;	// Just passed on
in vec3 worldCoord;	// World coordinates of the fragment
in vec3 vecToEye;	// Vector from vertex to eye
in vec3 position;

void main(){
	// Texture image data. This will be the base for the colors.
	vec4 baseFrag = texture2D(baseImage, UV_Coord);
	vec3 white = vec3(1,1,1);

	/// Assume eye is at 0,0,0 for now
	float distance = length(vecToEye);
	float intensity = 1.0f;
	intensity = 1.0f / (1.0f + distance);
	gl_FragColor = vec4(white * intensity, 1.0);
//	gl_FragColor = vec4(0,0,0,1);
	return;
}

/*
#version 150

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

// Input data from the fragment shader
in vec3 normal;		// Interpollated coordinates that have been transformed to view space
in vec2 UV_Coord;	// Just passed on
in vec3 worldCoord;	// World coordinates of the fragment
in vec3 vecToEye;	// Vector from vertex to eye

// Outs
out vec4 gl_FragColor;

void main(void) {

	// Texture image data. This will be the base for the colors.
	vec4 baseFrag = texture(baseImage, UV_Coord);
	
	if (!lightActive){
		gl_FragColor = baseFrag;
		return;
	}

	// Directional lighting ^^
	if (lightType == 0){
		// Diffuse
		vec3 lightDirection = normalize(lightDirOrPos.xyz);
		vec3 normalizedNormal = normalize(normal);
		float intensity = max(dot(lightDirection, normalizedNormal), 0);
		gl_FragColor += vec4(intensity * baseFrag.xyz, baseFrag.w) * lightDiffuse * materialDiffuse;


		// Specular - intensity same, but consider the half vector and eye position.
		vec3 vectorToEye = normalize(vecToEye);
		vec3 halfVector = normalize((lightDirection + vectorToEye) / 2);
		float initialBrightness = max(dot(halfVector, normalizedNormal), 0);
		int smoothness = 12;
		float totalBrightness = initialBrightness;
		for (int i = 0; i < materialShininess; ++i){
			totalBrightness *= initialBrightness;
		}
		gl_FragColor.xyz += totalBrightness * baseFrag.xyz * lightSpecular.xyz * materialSpecular.xyz;
	}
	// Positional lighting o-o;
	else if (lightType == 1){
		// Diffuse
		vec3 lightDirection = normalize(lightDirOrPos.xyz - worldCoord.xyz);
		vec3 normalizedNormal = normalize(normal);
		float intensity = max(dot(lightDirection, normalizedNormal), 0);
		gl_FragColor += vec4(intensity * baseFrag.xyz, baseFrag.w) * lightDiffuse * materialDiffuse;


		// Specular - intensity same, but consider the half vector and eye position.
		vec3 vectorToEye = normalize(vecToEye);
		vec3 halfVector = normalize((lightDirection + vectorToEye) / 2);
		float initialBrightness = max(dot(halfVector, normalizedNormal), 0);
		int smoothness = 12;
		float totalBrightness = initialBrightness;
		for (int i = 0; i < materialShininess; ++i){
			totalBrightness *= initialBrightness;
		}
		gl_FragColor.xyz += totalBrightness * baseFrag.xyz * lightSpecular.xyz * materialSpecular.xyz;
	}
	// Add ambient at the end
	vec3 ambient = vec3(0.1,0.1,0.1);
	gl_FragColor.xyz += ambient * baseFrag.xyz * lightAmbient.xyz * materialAmbient.xyz;
}

*/