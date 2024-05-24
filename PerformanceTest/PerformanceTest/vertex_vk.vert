#version 450
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;

layout(location = 0) out vec4 v_color;

layout(set = 0, binding = 0) uniform dynamic_matrices {
    mat4 m[1000];
} _dynamic_matrices;

void main() 
{
	v_color = a_color;
	gl_Position = _dynamic_matrices.model * vec4(a_position, 1.0f);
}