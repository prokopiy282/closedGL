#version 460 core
out vec4 fragmentColour;
in vec4 gl_FragCoord;
uniform vec2 windowSize;
uniform float shaderTime;
uniform float gammaCorrection;
uniform float colorSpeed;


void main() {
	fragmentColour = vec4(
	pow((gl_FragCoord.x/windowSize.x)*(sin(shaderTime*colorSpeed)), gammaCorrection), 
	pow((gl_FragCoord.y/windowSize.y)*(cos(shaderTime*colorSpeed)), gammaCorrection), 
	0.5f, 
	1.0f);
}