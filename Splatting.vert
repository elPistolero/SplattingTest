#version 330

in vec2 mu;
in vec3 cov;

out vec2 geoMu;
out vec3 geoCov;

void main () {
    //gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    
    geoMu = mu;
    geoCov = cov;
}