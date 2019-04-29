#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 tc;
uniform mat4 invProj;
uniform mat4 invView;
uniform vec2 iResolution;

out vec2 TexCoords;

void main(){

  gl_Position = vec4(pos, 0.0, 1.0);
  TexCoords = tc;
}