#shader vertex
#version 460

layout(location = 0) in vec4 vPosition;

layout(std140) uniform Matrices
{
    mat4 projection;
};

uniform mat4 model;
uniform vec2 size;

out vec2 fPos;
out vec2 fSize;

void main()
{
    vec4 tempPosition = vec4(vPosition.x * size.x, vPosition.y * size.y, vPosition.z, vPosition.w);
    gl_Position = projection * model * tempPosition;
    fPos = vec2(tempPosition);
    fSize = size;
}

#shader fragment
#version 460

uniform vec3 color;

in vec2 fPos;
in vec2 fSize;

out vec4 fColor;

void main()
{
    float dist;
    float alphaOutside;
    float alphaInside;
    if (fPos.y < fSize.y - fSize.x && fPos.y > fSize.x - fSize.y)
    {
        dist = distance(fPos, vec2(0.0, fPos.y));
        float delta = fwidth(dist);
        alphaOutside = smoothstep(fSize.x, fSize.x - delta, dist);
        alphaInside = smoothstep(fSize.x - 0.01, fSize.x - 0.01 + delta, dist);
    }
    else
    {
        if (fPos.y > 0)
        {
            dist = distance(fPos, vec2(0.0, fSize.y - fSize.x));
        }
        else
        {
            dist = distance(fPos, vec2(0.0, fSize.x - fSize.y));
        }
        float delta = fwidth(dist);
        alphaOutside = smoothstep(fSize.x, fSize.x - delta, dist);
        alphaInside = smoothstep(fSize.x - 0.01, fSize.x - 0.01 + delta, dist);
    }
    float alpha = alphaOutside * alphaInside;

    fColor = vec4(color, alpha);
}