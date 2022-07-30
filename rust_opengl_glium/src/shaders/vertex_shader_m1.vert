#version 150

in vec3 position;

out vec4 c;

uniform mat4 model;
uniform vec4 v_c;

void main() {
    c = v_c;
    gl_Position = model * vec4(position, 1.0);
}