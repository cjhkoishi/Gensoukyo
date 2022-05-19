#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 camera_transform;
uniform mat4 projection;

uniform samplerBuffer triangles;

void main(){
    gl_Position=projection*view*model*vec4(aPos,1);
}