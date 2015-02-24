#version 120
varying float r;
varying vec3 centre ;
uniform vec4 ogf_uniform_0;

void main(void) {
	float d = distance(gl_FragCoord.xy, centre.xy) ;
	if(d > r*0.5 ) discard;
	float z = sqrt(pow(r*0.5,2) - pow(d,2)) ;
	float z2 = ((gl_DepthRange.far - gl_DepthRange.near) / 2) / (z - (gl_DepthRange.far + gl_DepthRange.near) / 2);
	vec3 frag = vec3(gl_FragCoord.xy, z) ;
	gl_FragDepth = z2+centre.z ;
	vec3 l = vec3(gl_LightSource[0].position) ;
	vec3 n = frag - centre ;
	vec3 c = vec3 (0.0, 0.0, 5.0) ;
	vec3 r = reflect(-l, n) ;
	float diff = 0.2+max(0,dot(normalize(l), normalize(n)));
	float spec = pow(max(0, dot(normalize(c), normalize(r.xyz))), 100) ;
	gl_FragColor = gl_Color * (diff+spec) ;
}

