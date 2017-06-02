
// Input variable declarations
uniform sampler2D sColorTex;
varying vec2 vTexCoord;

// Per-fragment operations
void main()
{
	gl_FragColor = vec4(vec3(1.0) - texture(sColorTex, vTexCoord).rgb, 1.0);
}
