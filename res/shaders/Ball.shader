#shader vertex
#version 460

layout(location = 0) in vec4 vPosition;

layout(std140) uniform Matrices
{
    mat4 projection;
};

uniform mat4 model;
uniform float size;

out vec2 fPos;
out float fSize;

void main()
{
    vec4 tempPosition = vec4(vPosition.x * (size + .01), vPosition.y * (size + .01), vPosition.z, vPosition.w);
    gl_Position = projection * model * tempPosition;
    fPos = vec2(tempPosition);
    fSize = size;
}

#shader fragment
#version 460

uniform vec2 paddleVec;
uniform float paddleSize;

in vec2 fPos;
in float fSize;

out vec4 fColor;

void main()
{
    float dist = distance(fPos, vec2(0.0));
    float delta = fwidth(dist);
    float alphaOutside = smoothstep(fSize, fSize - delta, dist);
    float alphaInside = smoothstep(fSize - 0.01, fSize - 0.01 + delta, dist);

    fColor = vec4(1.0, 1.0, 1.0, alphaOutside * alphaInside);
}