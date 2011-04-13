#version 130

uniform mat2 cov;
uniform vec2 mu;

out vec4 frag_color;

void main () {
    //frag_color = vec4(1, 0, 0, 1);
    vec2 pos = mod(gl_FragCoord.xy, vec2(50.0)) - vec2(25.0);
    float dist_squared = dot(pos, pos);
    
    if ((dist_squared > 575.0) || (dist_squared < 100.0))
        discard;
        
    gl_FragColor = mix(vec4(.9, .9, .9, 1), vec4(.2, .2, .4, 1), smoothstep(380.25, 420.25, dist_squared));
}