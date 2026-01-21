#version 460 core
layout (location = 0) in vec3 vertexPosition;
//layout (location = 1) in vec2 texPosition;


uniform vec2 windowSize;
uniform mat4 transformationMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec2 TexCoord;


void main() {
	gl_Position = projectionMatrix * viewMatrix * transformationMatrix * vec4(vertexPosition.x, vertexPosition.y, vertexPosition.z, 1.0f);
	//TexCoord = texPosition;
	TexCoord = vec2(vertexPosition.x,vertexPosition.y);
}