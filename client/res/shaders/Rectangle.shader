#shader vertex
#version 460

layout(location = 0) in vec4 vPosition;

layout(std140) uniform Matrices {
    mat4 projection;
};

uniform mat4 model;
uniform vec2 size;

void main() {
    vPosition.xy *= size;
    gl_Position = projection * model * vPosition;
}

#shader fragment
#version 460

uniform vec3 color;

out vec4 fColor;

void main() {
    fColor = vec4(color, 1.0);
}