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
    vPosition.xy *= (size + .01);
    gl_Position = projection * model * vPosition;
    vModelPosition = vPosition.xy;
    vModelSize = size;
}

#shader fragment
#version 460

uniform vec3 color;
uniform float stamina;
uniform vec2 paddleVec;
uniform float paddleSize;

in vec2 vModelPosition;
in float vModelSize;

out vec4 fColor;

void main() {
    float dist = distance(vModelPosition, vec2(0.0));
    float delta = fwidth(dist);
    float alpha = 0.0;

    if (dist > vModelSize / 2) {
        float alphaOutside = smoothstep(vModelSize, vModelSize - delta, dist);
        float alphaInside = smoothstep(vModelSize - 0.01, vModelSize - 0.01 + delta, dist);
        alpha = alphaOutside * alphaInside;
    } else if (dist > vModelSize / 10) {
        if (atan(vModelPosition.y, vModelPosition.x) < stamina) {
            float alphaOutside = smoothstep(vModelSize / 4, (vModelSize / 4) - delta, dist);
            float alphaInside = smoothstep((vModelSize / 4) - 0.0075, (vModelSize / 4) - 0.0075 + delta, dist);
            alpha = alphaOutside * alphaInside;
        }
    } else {
        if (atan(vModelPosition.y, vModelPosition.x) < stamina) {
            alpha = smoothstep(vModelSize / 10, (vModelSize / 10) - delta, dist);
        }
    }

    vec3 outColor = vec3(1.0, 1.0, 1.0);
    if (distance(vModelPosition, paddleVec * vModelSize) <= paddleSize) {
        outColor = color;
    }

    fColor = vec4(tempColor, alpha);
}