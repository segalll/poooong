#shader vertex
#version 460

layout(location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>

layout(std140) uniform Matrices {
    mat4 projection;
};

out vec2 texCoords;

void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    texCoords = vertex.zw;
}

#shader fragment
#version 460

in vec2 texCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main() {
    color = vec4(textColor, texture(text, texCoords).r);
}