#version 460 core
layout (location = 0) in vec3 vertexPosition;

uniform mat4 transformationMatrix;


void main() {
	gl_Position = transformationMatrix * vec4(vertexPosition.x, vertexPosition.y, vertexPosition.z, 1.0);
}