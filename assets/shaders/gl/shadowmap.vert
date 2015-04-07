#version 330 core
 
// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texcoord;
 
uniform mat4 projectionMatrix;
uniform mat4 cameraMatrix;
uniform mat4 modelMatrix;

varying vec3 pos;
varying vec2 texcoord;

 
void main(){
	texcoord = a_texcoord;
	pos = vec3(modelMatrix * vec4(a_position,1));
	gl_Position =  projectionMatrix * cameraMatrix * modelMatrix * vec4(a_position,1);
}