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

uniform float circlePos;

in vec2 fPos;
in vec2 fSize;

out vec4 fColor;

void main()
{
    float used = (circlePos - 0.5) * 2 * (fSize.x - fSize.y);
    float dist = distance(fPos, vec2(used, 0.0));
    float delta = fwidth(dist);
    float alphaOutside = smoothstep(fSize.y, fSize.y - delta, dist);
    float alphaInside = smoothstep(fSize.y - 0.01, fSize.y - 0.01 + delta, dist);
    float alpha = 0.0;
    if (abs(fPos.x - used) > fSize.y)
    {
        if (abs(fPos.y) < fSize.y / 4)
        {
            alpha = 1.0;
        }
    }
    else
    {
        alpha = alphaOutside * alphaInside;
    }

    fColor = vec4(1.0, 1.0, 1.0, alpha);
}