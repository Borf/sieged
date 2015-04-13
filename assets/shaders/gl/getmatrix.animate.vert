#version 150

uniform mat4 modelMatrix;
uniform mat4 boneMatrices[50];

in ivec4 a_boneIds;
in vec4 a_boneWeights;


mat4 getModelMatrix()
{
	return modelMatrix * boneMatrices[a_boneIds.x];
}