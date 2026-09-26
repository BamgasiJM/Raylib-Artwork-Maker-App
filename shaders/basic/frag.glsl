#version 330

out vec4 finalColor;

uniform float u_time;

void main()
{
    float r = sin(u_time) * 0.5 + 0.5;
    float g = sin(u_time + 2.094) * 0.5 + 0.5;
    float b = sin(u_time + 4.188) * 0.5 + 0.5;

    vec3 color = vec3(r, g, b);

    finalColor = vec4(color, 1.0);
}