#version 330

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

in vec2 mu;
in vec3 cov;

out vec3 geoCov;

void main () {
    //gl_Position = projectionMatrix * modelViewMatrix * vec4(mu, 0, 1);
    gl_Position = vec4(mu, 0, 1);
    geoCov = cov;
}