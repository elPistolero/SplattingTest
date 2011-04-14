#version 130

in vec2 quadVert;
in vec3 s;
in vec3 c;
in vec3 mu;
in float weight;

void main () {
    //gl_Position = gl_ModelViewProjectionMatrix * vec4(vVertex, 1);
    if (s.x == 1) {
        gl_Position = gl_ModelViewProjectionMatrix * vec4(quadVert, -1, 1);
    } else {
        gl_Position = vec4(0, 0, 0, 0);
    }
}