#version 460 core
out vec4 fragmentColour;
in vec4 gl_FragCoord;
uniform vec2 WindowSize;


void main() {
	fragmentColour = vec4(gl_FragCoord.x/WindowSize.x, gl_FragCoord.y/WindowSize.y, 0.5f, 1.0f);
}