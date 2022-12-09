#shader vertex
#version 460

layout(location = 0) in vec4 vPosition;

layout(std140) uniform Matrices {
    mat4 projection;
};

uniform mat4 model;
uniform vec2 size;

out vec2 vModelPosition;
out vec2 vModelSize;

void main() {
    vPosition *= size;
    gl_Position = projection * model * vPosition;
    vModelPosition = vPosition.xy;
    vModelSize = size;
}

#shader fragment
#version 460

uniform float circlePos;

in vec2 vModelPosition;
in vec2 vModelSize;

out vec4 fColor;

void main() {
    float used = (circlePos - 0.5) * 2 * (vModelSize.x - vModelSize.y);
    float dist = distance(vModelPosition, vec2(used, 0.0));
    float delta = fwidth(dist);
    float alphaOutside = smoothstep(vModelSize.y, vModelSize.y - delta, dist);
    float alphaInside = smoothstep(vModelSize.y - 0.01, vModelSize.y - 0.01 + delta, dist);
    float alpha = 0.0;
    if (abs(vModelPosition.x - used) > vModelSize.y) {
        if (abs(vModelPosition.y) < vModelSize.y / 4) {
            alpha = 1.0;
        }
    } else {
        alpha = alphaOutside * alphaInside;
    }

    fColor = vec4(1.0, 1.0, 1.0, alpha);
}