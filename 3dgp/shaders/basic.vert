#version 330

// Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;

// Lights
struct AMBIENT
{	int on;
	vec3 color;
};
struct DIRECTIONAL
{	int on;
	vec3 direction;
	vec3 diffuse;
};

uniform AMBIENT lightAmbient;
uniform DIRECTIONAL lightDir;
uniform float fogDensity;

layout (location = 0) in vec3 aVertex;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;
layout (location = 4) in vec3 aTangent;
layout (location = 5) in vec3 aBiTangent;

// Output (sent to Fragment Shader)
out vec4 color;
out vec4 position;
out vec3 normal;
out mat3 matrixTangent;
out vec2 texCoord0;
out vec2 texCoord1;
out vec2 texCoord2;
out float fogFactor;

vec4 compAmbient(vec3 material, AMBIENT light)
{
	return vec4(material * light.color, 1);
}

vec4 compDirectional(vec3 material, DIRECTIONAL light)
{
	vec3 L = normalize(mat3(matrixView) * light.direction).xyz;
	float NdotL = dot(normal.xyz, L);
	if (NdotL > 0)
		return vec4(light.diffuse * material * NdotL, 1);
	else
		return vec4(0, 0, 0, 1);
}

void main(void) 
{
	// calculate position & normal
	position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;
	normal = normalize(mat3(matrixModelView) * aNormal);

	// calculate the colour
	color = vec4(0, 0, 0, 0);

	// calculate tangent local system transformation
	vec3 tangent = normalize(mat3(matrixModelView) * aTangent);
	tangent = normalize(tangent - dot(tangent, normal) * normal);	// Gramm-Schmidt process
	vec3 biTangent = cross(normal, tangent);
	matrixTangent = mat3(tangent, biTangent, normal);
	

	// ambient & emissive light
	if (lightAmbient.on == 1)
		color += compAmbient(materialAmbient, lightAmbient);

	// directional lights
	if (lightDir.on == 1)
		color += compDirectional(materialDiffuse, lightDir);

	// calculate texture coordinate
	texCoord0 = aTexCoord;
	texCoord1 = aTexCoord;
	texCoord2 = aTexCoord;

	//calculating fog factor
	fogFactor = exp2(-fogDensity * length(position));
}
