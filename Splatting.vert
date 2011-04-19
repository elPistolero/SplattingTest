#version 130

uniform mat4x3 viewport;
uniform mat4 modelView;
uniform mat4 projection;

in vec3 quadVert;
in vec3 s;
in vec3 c;
in vec3 mu;
in float weight;

out vec3 fragMu;
out mat2 fragV;
out mat2 fragQ;
out float fragDetV;

void main () {
    float trace = s.x + s.y;
    float det = s.x*s.y - c.x * c.x;
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
    
    mat4 mvp = projection * modelView;
    
    if (quadVert.z == 0) {
       vec2 e1 = normVec1 * val1;
       vec2 e2 = normVec2 * val2; 
       vec2 c1 = mu.xy + e1 + e2;
       gl_Position = mvp * vec4(c1, 0, 1);
    } else if (quadVert.z == 1) {
       vec2 e1 = normVec1 * val1;
       vec2 e4 = -normVec2 * val2; 
       vec2 c2 = mu.xy + e1 + e4;
       gl_Position = mvp * vec4(c2, 0, 1);
    } else if (quadVert.z == 2) {
       vec2 e3 = -normVec1 * val1;
       vec2 e4 = -normVec2 * val2; 
       vec2 c3 = mu.xy + e3 + e4;
       gl_Position = mvp * vec4(c3, 0, 1);
    } else if (quadVert.z == 3) {
       vec2 e2 = -normVec1 * val1;
       vec2 e3 = normVec2 * val2; 
       vec2 c4 = mu.xy + e2 + e3;
       gl_Position = mvp * vec4(c4, 0, 1);
    }
    
    // object coordinates -(modelView matrix)-> eye coordinates
    // eye coordinates -(projection matrix)-> clip coordinates
    // clip coordinates -(dehomogenize)-> normalized device coordinates
    // normalized device coordinates -(viewport transformation)-> window coordinates
    // viewport transformation: http://www.songho.ca/opengl/gl_transform.html
    // glViewport(x,y,w,h);
    // glDepthRange(n,f);
    //  x = (w/2)*x_ndc + (x + w/2);
    //  y = (h/2)*y_ndc + (y + h/2);
    //  z = ((f-n)/2)*z_ndc + (f+n)/2;
    
    float x = 0;
    float y = 0;
    float width = 1600;
    float height = 1200;
    float n = -1;
    float f = 1;
    vec4 clipCoord = projection * modelView * vec4(mu.xy, 0, 1);
    vec3 ndc = vec3(clipCoord.x/clipCoord.w, clipCoord.y/clipCoord.w, clipCoord.z/clipCoord.w);
    fragMu.x = (width/2)*ndc.x + (x + width/2);
    fragMu.y = (height/2)*ndc.y + (y + height/2);
    fragMu.z = ((f-n)/2)*ndc.z + ((f+n)/2);
    fragV = V;
    fragDetV = det;
    fragQ = Q;
}