#version 150

attribute vec3 a_position;
attribute vec2 a_texcoord;
attribute vec3 a_normal;

uniform mat4 projectionMatrix;
uniform mat4 cameraMatrix;
uniform mat4 modelMatrix;

uniform mat4 shadowCameraMatrix;
uniform mat4 shadowProjectionMatrix;


varying vec2 texcoord;
varying vec3 normal;
varying vec3 pos;
varying vec3 shadowPos;
varying vec3 LightDirection_cameraspace;


void main()
{
mat4 biasMatrix = mat4(
0.5, 0.0, 0.0, 0.0,
0.0, 0.5, 0.0, 0.0,
0.0, 0.0, 0.5, 0.0,
0.5, 0.5, 0.5, 1.0
);

	texcoord = a_texcoord;

	mat3 normalMatrix = mat3(modelMatrix);
	normalMatrix = transpose(inverse(normalMatrix));
	normal = normalMatrix * a_normal;


	LightDirection_cameraspace = (cameraMatrix * vec4(0.5, 2.0, 2.0,0)).xyz;

	pos = vec3(modelMatrix * vec4(a_position,1));

	shadowPos = vec3(biasMatrix * shadowProjectionMatrix * shadowCameraMatrix * modelMatrix * vec4(a_position, 1));
	gl_Position = projectionMatrix * cameraMatrix * modelMatrix * vec4(a_position,1);
}