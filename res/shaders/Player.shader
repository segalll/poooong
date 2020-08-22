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

uniform vec3 color;
uniform float stamina;
uniform vec2 paddleVec;
uniform float paddleSize;

in vec2 fPos;
in float fSize;

out vec4 fColor;

void main()
{
    float dist = distance(fPos, vec2(0.0));
    float delta = fwidth(dist);
    float alpha = 1.0;

    if (dist > fSize / 2)
    {
        float alphaOutside = smoothstep(fSize, fSize - delta, dist);
        float alphaInside = smoothstep(fSize - 0.01, fSize - 0.01 + delta, dist);
        alpha = alphaOutside * alphaInside;
    }
    else if (dist > fSize / 10)
    {
        if (atan(fPos.y, fPos.x) < stamina)
        {
            float alphaOutside = smoothstep(fSize / 4, (fSize / 4) - delta, dist);
            float alphaInside = smoothstep((fSize / 4) - 0.0075, (fSize / 4) - 0.0075 + delta, dist);
            alpha = alphaOutside * alphaInside;
        }
        else
        {
            alpha = 0.0;
        }
    }
    else
    {
        if (atan(fPos.y, fPos.x) < stamina)
        {
            alpha = smoothstep(fSize / 10, (fSize / 10) - delta, dist);
        }
        else
        {
            alpha = 0.0;
        }
    }

    vec3 tempColor = vec3(1.0, 1.0, 1.0);
    if (distance(fPos, paddleVec * fSize) <= paddleSize)
    {
        tempColor = color;
    }

    fColor = vec4(tempColor, alpha);
}