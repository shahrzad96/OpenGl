#version 330

in vec4 color;
out vec4 outColor;
in vec4 position;
in vec3 normal;
in float fogFactor;
in mat3 matrixTangent;

// Matrices
uniform mat4 matrixView;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;
uniform vec3 fogColour;


//Textures
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D textureNormal;

//Uniform
uniform float time;
uniform float speedX;
uniform float speedY;

//Inputs (from the vertex shader)
in vec2 texCoord0;
in vec2 texCoord1;
in vec2 texCoord2;

//global variable
vec3 normalNew;

struct SPOT
{
	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	vec3 direction;
	float cutoff; 
	float attenuation;
	mat4 matrix;
};
struct POINT
{
	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};


uniform POINT lightPoint1;

//point light

vec4 PointLight(POINT light)
{
	// Calculate Point Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = (normalize(matrixView * vec4(light.position, 1) - position)).xyz;
	float NdotL = dot(normalNew, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;

	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normalNew);
	float RdotV = dot(R, V);

	if (NdotL > 0 && RdotV > 0)
	    color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	// attenuation
	float dist = length(matrixView * vec4(light.position, 1) - position);
	float att = 1 / (dist * dist) / 0.1;

	return color * att;
}

//creating spot light


uniform SPOT lightSpot;

vec4 SpotLight(SPOT light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);

	// diffuse light
	vec3 L = normalize(light.matrix * vec4(light.position, 1) - position).xyz;
	float NdotL = dot(normalNew, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;

	// specular light
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normalNew);
	float RdotV = dot(R, V);
	if (NdotL > 0 && RdotV > 0)
	    color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	// spot angles and the Spot Factor
	vec3 D = mat3(matrixView) * light.direction;
	float spotFactor = dot(-L, D);
	float angle = acos(spotFactor);
	float cutoff = radians(clamp(light.cutoff, 0, 90));
	if (angle <= cutoff)
		spotFactor = pow(spotFactor, light.attenuation);
	else
		spotFactor = 0;


	return color * spotFactor;
}



void main(void) 
{
	//animating texture
	float xScrollValue = speedX * time;
	float yScrollValue = speedY * time;
	vec2 offsetVec = vec2(xScrollValue, yScrollValue);

	//setting texture
	outColor = color;
	outColor *= texture(texture0, texCoord0);
	outColor *= texture(texture1, texCoord1);
	outColor *= texture(texture2, texCoord2 + offsetVec);
	outColor *= texture(textureNormal, texCoord0);

	//setting normal map
	normalNew = 2.0 * texture(textureNormal, texCoord0).xyz - vec3(1.0, 1.0, 1.0);
	normalNew = normalize(matrixTangent * normalNew);

	
	//adding lights to the scene
	if (lightPoint1.on == 1) 
		outColor += PointLight(lightPoint1);
	
	//adding fog to the scene
	outColor = mix(vec4(fogColour, 1), outColor, fogFactor);
}
