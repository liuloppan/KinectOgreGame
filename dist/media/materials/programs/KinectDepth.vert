
// Input and output variable declarations
uniform mat4x4 mWorldViewProjMatrix;
varying vec2 vTexCoord;

// Per-vertex operations
void main()
{
	gl_Position = mWorldViewProjMatrix * gl_Vertex;
	vTexCoord = gl_MultiTexCoord0.st;
}
