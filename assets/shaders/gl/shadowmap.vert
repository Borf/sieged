#version 330 core
 
// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 a_position;
 
// Values that stay constant for the whole mesh.
uniform mat4 projectionMatrix;
uniform mat4 cameraMatrix;
uniform mat4 modelMatrix;
 
void main(){
 gl_Position =  projectionMatrix * cameraMatrix * modelMatrix * vec4(a_position,1);
}