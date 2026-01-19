#version 460 core
layout (location = 0) in vec3 vertexPosition;


uniform vec2 windowSize;
uniform mat4 transformationMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;




void main() {
	gl_Position = projectionMatrix * viewMatrix * transformationMatrix * vec4(vertexPosition.x, vertexPosition.y, vertexPosition.z, 1.0f);
}