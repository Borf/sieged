#version 150

attribute vec3 a_position;
attribute vec2 a_texcoord;
attribute vec3 a_normal;

uniform mat4 projectionMatrix;
uniform mat4 cameraMatrix;

uniform mat4 shadowCameraMatrix;
uniform mat4 shadowProjectionMatrix;
uniform vec3 lightDirection;

varying vec2 texcoord;
varying vec3 normal;
varying vec3 pos;
varying vec4 shadowPos;
varying vec3 LightDirection_cameraspace;

mat4 biasMatrix = mat4(
0.5, 0.0, 0.0, 0.0,
0.0, 0.5, 0.0, 0.0,
0.0, 0.0, 0.5, 0.0,
0.5, 0.5, 0.5, 1.0
);


mat4 getModelMatrix();

void main()
{

	texcoord = a_texcoord;

	mat4 modelMatrix = getModelMatrix();

	mat3 normalMatrix = mat3(modelMatrix);
	normalMatrix = transpose(inverse(normalMatrix));
	normal = normalMatrix * a_normal;


	LightDirection_cameraspace = (cameraMatrix * vec4(lightDirection,0)).xyz;

	pos = vec3(modelMatrix * vec4(a_position,1));

	shadowPos = biasMatrix * shadowProjectionMatrix * shadowCameraMatrix * modelMatrix * vec4(a_position, 1);
	gl_Position = projectionMatrix * cameraMatrix * modelMatrix * vec4(a_position,1);
}