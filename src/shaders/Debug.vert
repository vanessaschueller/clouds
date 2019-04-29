#version 330 core

layout (location = 0) in vec4 Position;
layout (location = 1) in vec3 Normal;


uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out mat4 modelViewProjection;

out Vertex
{
	vec4 normal;
	vec4 color;
} vertex;


void main()
{	

	vertex.color = vec4(1.0,1.0,0.0,1.0);
	vertex.normal = vec4(Normal,1.0);
    

	gl_Position = Position;

}
