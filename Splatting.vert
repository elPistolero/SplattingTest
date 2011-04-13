#version 130

uniform mat2 cov;
uniform vec2 mu;

in vec3 vVertex;

void main () {
    gl_Position = gl_ModelViewProjectionMatrix * vec4(vVertex, 1);
    gl_PointSize = 80; // in pixels height and width
}