#version 450
layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec4 v_color;

void main() 
{
	o_Color = v_color;
}