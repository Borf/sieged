#version 330 core
 
// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texcoord;
 
uniform mat4 projectionMatrix;
uniform mat4 cameraMatrix;

varying vec3 pos;
varying vec2 texcoord;

mat4 getModelMatrix();


void main(){
	mat4 mm = getModelMatrix();
	texcoord = a_texcoord;
	pos = vec3(mm * vec4(a_position,1));
	gl_Position =  projectionMatrix * cameraMatrix * mm * vec4(a_position,1);
}