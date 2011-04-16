#version 130

in vec3 quadVert;
in vec3 s;
in vec3 c;
in vec3 mu;
in float weight;

void main () {
    if (weight == 1) {
        gl_Position = gl_ModelViewProjectionMatrix * vec4(quadVert.xy, 0, 1);
    } else {
        gl_Position = vec4(0, 0, 0, 0);
    }
}