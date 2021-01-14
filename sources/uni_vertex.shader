#version 420 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 v_color;


uniform mat4 MVP;

void main()
{
	gl_Position = MVP *  position;
};
