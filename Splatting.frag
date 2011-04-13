#version 130

uniform mat2 cov;
uniform vec2 mu;

out vec4 frag_color;

void main () {
    frag_color = vec4(1, 0, 0, 1);
}