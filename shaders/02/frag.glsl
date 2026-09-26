#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform float u_time;

out vec4 finalColor;

void main()
{
    // fragTexCoord를 -1~1 범위로 정규화 (중앙 기준)
    vec2 uv = (fragTexCoord - 0.5) * 2.0;

    // 시간 기반 색상 변화
    float r = sin(u_time * 1.5 + uv.x * 3.0) * 0.5 + 0.5;
    float g = sin(u_time * 1.2 + uv.y * 3.0 + 2.094) * 0.5 + 0.5;
    float b = sin(u_time * 0.8 + (uv.x + uv.y) * 3.0 + 4.188) * 0.5 + 0.5;

    finalColor = vec4(r, g, b, 1.0);
}