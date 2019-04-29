#pragma once


#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/transform.hpp>
#include <Camera/Camera.h>
#include <Shader/Shader.h>
#include <Texture/Texture.h>
#include <Shader/ComputeShader.h>



#define INVALID_GL_VALUE 0xFFFFFFFF
