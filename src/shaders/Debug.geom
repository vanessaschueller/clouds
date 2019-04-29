#version 150
layout(triangles) in;

layout(line_strip, max_vertices = 2) out;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform int  vectorLength;
in           Vertex
{
    vec4  normal;
    vec4  color;
}
vertex[];

out vec4 vertex_color;

void main()
{
    int i;
    for(i = 0; i < gl_in.length(); i++)
    {
        vec3 P = gl_in[i].gl_Position.xyz;
        vec3 N = vertex[i].normal.xyz;

        gl_Position  = projectionMatrix * viewMatrix * modelMatrix * vec4(P + N * vectorLength, 1.0);
        vertex_color = vec4(1.0,.0,.0, 1.0);
        EmitVertex();

        gl_Position  = projectionMatrix * viewMatrix * modelMatrix * vec4(P, 1.0);
        vertex_color = vertex[i].color;
        EmitVertex();

        EndPrimitive();
    }
}
