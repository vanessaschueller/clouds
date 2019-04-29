#version 330

in vec2 TexCoords;

uniform sampler2D previousframe;

out vec4 pixelColor;

void main()
{

    pixelColor = texture(previousframe,TexCoords); 
}
