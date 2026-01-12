#version 460 core
layout (location = 0) in vec3 vertexPosition;


uniform vec2 windowSize;
uniform mat4 transformationMatrix;
uniform mat4 projectionMatrix;




void main() {
	gl_Position = projectionMatrix * transformationMatrix * vec4(vertexPosition.x, vertexPosition.y, vertexPosition.z, 1.0);
}