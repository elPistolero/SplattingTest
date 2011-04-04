#version 330
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

layout(points) in;
in vec3 geoCov[];

layout(line_strip, max_vertices = 5) out;

void main() {
    //gl_Position = vec4(0, 0, 0, 0);
    
    // calculate the bounding box
    float trace = geoCov[0].x + geoCov[0].y;
    float det = geoCov[0].x * geoCov[0].y - pow(geoCov[0].z, 2);
    float eigenVal1 = (trace + sqrt(pow(trace, 2) - 4 * det))/2;
    float eigenVal2 = (trace - sqrt(pow(trace, 2) - 4 * det))/2;
    
    // length of each axis of the ellipse
    float lengthX = sqrt(eigenVal1);
    float lengthY = sqrt(eigenVal2);    
    
    // orientation of the first eigenvector in relation to the x-axis
    float angleDeg = 0;
    if (geoCov[0].z != 0) {
        vec3 eigenVec1 = vec3(eigenVal1 - geoCov[0].y, geoCov[0].z, 0);
        float angleRad = acos(dot(eigenVec1, vec3(0, 1, 0))/length(eigenVec1));
        angleDeg = degrees(angleRad);
    }
    
    mat4 modelViewProjection = projectionMatrix * modelViewMatrix;
    
    gl_Position = modelViewProjection * (gl_in[0].gl_Position + vec4(-lengthX, -lengthY, 0, 0));
    EmitVertex();
    gl_Position = modelViewProjection * (gl_in[0].gl_Position + vec4(lengthX, -lengthY, 0, 0));
    EmitVertex();
    gl_Position = modelViewProjection * (gl_in[0].gl_Position + vec4(lengthX, lengthY, 0, 0));
    EmitVertex();
    gl_Position = modelViewProjection * (gl_in[0].gl_Position + vec4(-lengthX, lengthY, 0, 0));
    EmitVertex();
    gl_Position = modelViewProjection * (gl_in[0].gl_Position + vec4(-lengthX, -lengthY, 0, 0));
    EmitVertex();
    
    EndPrimitive();
}