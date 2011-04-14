#version 130

uniform mat2 cov;
uniform vec2 mu;

in vec2 quadVert;

void main () {
    //gl_Position = gl_ModelViewProjectionMatrix * vec4(vVertex, 1);
    gl_Position = gl_ModelViewProjectionMatrix * vec4(quadVert, -1, 1);
}