#version 130

in vec3 fragMu;
in mat2 fragV;
in float fragDetV;
in mat2 fragQ;

out vec4 frag_color;

void main () {
    const float PI = 3.14159265358979323846264;
	vec2 dis = gl_FragCoord.xy - fragMu.xy;
	vec2 tmp = fragQ * dis;
	float r = dis.x*tmp.x + dis.y*tmp.y;

	if (r < 400) {
	   float footprint = 1/(2*PI) * inversesqrt(fragDetV) * exp(-r/2);
	   frag_color = vec4(1/footprint, 1/footprint, 1/footprint, 1);
	} else {
	  frag_color = vec4(0, 0, 1, 1);
	}
}
