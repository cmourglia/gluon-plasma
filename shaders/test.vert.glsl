#version 450

layout (location = 0) in vec2 in_Position;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

void main()
{
    gl_Position = u_Proj * u_View * u_Model * vec4(in_Position, 0, 1);
}
