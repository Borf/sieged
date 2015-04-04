attribute vec3 a_position;
attribute vec4 a_texcoord;
attribute vec3 a_normal;

uniform mat4 projectionMatrix;
uniform mat4 cameraMatrix;
uniform mat4 modelMatrix;

varying vec2 texcoord;
varying vec3 normal;
varying vec3 pos;


void main()
{
	texcoord = a_texcoord;

	mat3 normalMatrix = mat3(modelMatrix);
	normalMatrix = transpose(inverse(normalMatrix));
	normal = normalMatrix * a_normal;

	pos = vec3(modelMatrix * vec4(a_position,1));

	gl_Position = projectionMatrix * cameraMatrix * modelMatrix * vec4(a_position,1);
}