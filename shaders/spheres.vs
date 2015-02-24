#version 120
uniform vec4 ogf_uniform_0; // viewport
attribute float attr;

varying float r;
varying vec3 centre ;

void main() {
	r = attr * min(ogf_uniform_0.z - ogf_uniform_0.x, ogf_uniform_0.w - ogf_uniform_0.y) ;
	gl_FrontColor = gl_Color ;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_PointSize = r ;
	centre.z = gl_Position.z/gl_Position.w ; //((gl_DepthRange.far - gl_DepthRange.near) / 2) * gl_Position.z/gl_Position.w + (gl_DepthRange.far + gl_DepthRange.near) / 2;
	centre.xy = ogf_uniform_0.xy + ogf_uniform_0.zw/2 + ((gl_Position.xy/gl_Position.w) * ogf_uniform_0.zw/2) ;
}

