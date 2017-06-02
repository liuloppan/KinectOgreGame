
// Input variable declarations
uniform sampler2D sDepthTex;
varying vec2 vTexCoord;

// Per-fragment operations
void main()
{
	float fDepth = texture(sDepthTex, vTexCoord).r;
	gl_FragColor = vec4(10.0 * fDepth, 1.0 - 40.0 * fDepth, 20.0 * fDepth - 1.0, 1.0);
}
