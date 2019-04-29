#pragma once

#include <Includes/Defs.h>

class ScreenFillingQuad
{
public:
	ScreenFillingQuad(std::string pathToFragmentShader);
	~ScreenFillingQuad();
	void drawQuad();
	Shader getShader();

private:
	void initializeQuad();
	std::unique_ptr<Shader> m_shader;
	unsigned int m_quadVAO, m_quadVBO;
	bool m_initialized;
};