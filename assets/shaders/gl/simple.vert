attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec4 a_color;

uniform mat4 projectionMatrix;
uniform mat4 cameraMatrix;
uniform mat4 modelMatrix;

varying vec4 color;
varying vec3 normal;
varying vec3 pos;


void main()
{
	color = a_color;

	mat3 normalMatrix = mat3(modelMatrix);
	normalMatrix = transpose(inverse(normalMatrix));
	normal = normalMatrix * a_normal;

	pos = vec3(modelMatrix * vec4(a_position,1));

	gl_Position = projectionMatrix * cameraMatrix * modelMatrix * vec4(a_position,1);
}