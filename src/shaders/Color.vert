#version 330

layout(location = 0) in vec4 inputPosition;
layout(location = 1) in vec3 inputNormal;
layout(location = 2) in vec2 inputTexCoord;

uniform mat4 projection, model, view;
uniform vec3 lightPos;

out vec3 normalInterp;
out vec3 vertPos;
out vec3 lightCam;
out vec3 worldPos;
out vec2 TexCoords;

void main()
{
    gl_Position   = projection * view * model * inputPosition;
    vec4 vertPos4 = view * model * inputPosition;
    vertPos       = vec3(vertPos4) / vertPos4.w;
    normalInterp  = vec3(transpose(inverse(view * model)) * vec4(inputNormal, 0.0));
    lightCam      = (view * vec4(lightPos, 1.0)).xyz;
	worldPos = (model * inputPosition).xyz;
	TexCoords = inputTexCoord;
}
