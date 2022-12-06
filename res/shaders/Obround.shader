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
    vPosition.xy *= size;
    gl_Position = projection * model * vPosition;
    vModelPosition = vPosition.xy;
    vModelSize = size;
}

#shader fragment
#version 460

uniform vec3 color;

in vec2 vModelPosition;
in vec2 vModelSize;

out vec4 fColor;

float calcAlpha(vec2 pos, vec2 size) {
    float dist;
    if (pos.y < size.y - size.x && pos.y > size.x - size.y) {
        dist = distance(pos, vec2(0.0, pos.y));
    } else {
        dist = distance(pos, vec2(0.0, abs(size.y - size.x)));
    }

    float delta = fwidth(dist);
    float alphaOutside = smoothstep(size.x, size.x - delta, dist);
    float alphaInside = smoothstep(size.x - 0.01, size.x - 0.01 + delta, dist);

    return alphaOutside * alphaInside;
}

void main() {
    float alpha = calcAlpha(vModelPosition, vModelSize);

    fColor = vec4(color, alpha);
}