#version 450

in vec2 TexCoords;

out vec4 fragmentColor;



void main()
{

    vec4 mixed               = vec4(0.0, 0.1, 0.2, 0.3);
	vec3 gradient = vec3(smoothstep(mixed.x, mixed.y, TexCoords.y) - smoothstep(mixed.z, mixed.w, TexCoords.y));
    fragmentColor =  vec4(gradient,1);
}
