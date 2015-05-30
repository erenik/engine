// Author: Emil Hedemalm
// Date: 2015-02-20
// Very light-weight shadow mapping shader.
#version 120

void main()
{
	/// Set depth of the pixel as it was!
	gl_FragDepth = gl_FragCoord.z;
}

