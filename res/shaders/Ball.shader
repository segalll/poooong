#shader vertex
#version 460

layout(location = 0) in vec4 vPosition;

layout(std140) uniform Matrices {
    mat4 projection;
};

uniform mat4 model;
uniform float size;

out vec2 vModelPosition;
out float vModelSize;

void main() {
    position.xy *= (size + .01);
    gl_Position = projection * model * vPosition;
    vModelPosition = vPosition.xy;
    vModelSize = size;
}

#shader fragment
#version 460

uniform vec2 paddleVec;
uniform float paddleSize;

in vec2 vModelPosition;
in float vModelSize;

out vec4 fColor;

float calcAlpha(vec2 pos, vec2 size) {
    float dist = distance(pos, vec2(0.0));
    float delta = fwidth(dist);
    float alphaOutside = smoothstep(size, size - delta, dist);
    float alphaInside = smoothstep(size - .01, size - .01 + delta, dist);

    return alphaOutside * alphaInside;
}

void main() {
    float alpha = calcAlpha(vModelPosition, vModelSize);

    fColor = vec4(1.0, 1.0, 1.0, alpha);
}