#version 130

in vec3 quadVert;
in vec3 s;
in vec3 c;
in vec3 mu;
in float weight;

void main () {
    float trace = s.x + s.y;
    float det = s.x*s.y - pow(c.x, 2);
    mat2 V = mat2(s.x, c.x, c.x, s.y);
    mat2 Q = mat2(s.y, -c.x, -c.x, s.x) * 1/det; 
    
    float val1 = (trace + sqrt(pow(trace, 2) - 4*det))/2;
    float val2 = (trace - sqrt(pow(trace, 2) - 4*det))/2;
    
    vec2 eigenVec1;
    vec2 eigenVec2;
    vec2 normVec1;
    vec2 normVec2;
    if (c.x != 0) {
        eigenVec1 = vec2(val1 - s.y, c.x);
        eigenVec2 = vec2(val2 - s.y, c.x);
        normVec1 = normalize(eigenVec1);
        normVec2 = normalize(eigenVec2);
    } else {
        eigenVec1 = vec2(1, 0);
        eigenVec2 = vec2(0, 1);
        normVec1 = eigenVec1;
        normVec2 = eigenVec2;
    }
    
    if (quadVert.z == 0) {
       vec2 e1 = normVec1 * val1;
       vec2 e2 = normVec2 * val2; 
       vec2 c1 = mu.xy + e1 + e2;
       gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(c1, -1, 1);
    } else if (quadVert.z == 1) {
       vec2 e1 = normVec1 * val1;
       vec2 e4 = -normVec2 * val2; 
       vec2 c2 = mu.xy + e1 + e4;
       gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(c2, -1, 1);
    } else if (quadVert.z == 2) {
       vec2 e3 = -normVec1 * val1;
       vec2 e4 = -normVec2 * val2; 
       vec2 c3 = mu.xy + e3 + e4;
       gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(c3, -1, 1);
    } else if (quadVert.z == 3) {
       vec2 e2 = -normVec1 * val1;
       vec2 e3 = normVec2 * val2; 
       vec2 c4 = mu.xy + e2 + e3;
       gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(c4, -1, 1);
    }
}