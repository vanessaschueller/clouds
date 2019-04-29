#include <Shader/ScreenFillingQuad.h>

ScreenFillingQuad::ScreenFillingQuad(std::string pathToFragmentShader)
{
	m_quadVAO = INVALID_GL_VALUE;
	m_quadVBO = INVALID_GL_VALUE;
	m_initialized = false;
	initializeQuad();
	m_shader = std::make_unique<Shader>();
	m_shader->initFromFiles(CLOUDS_SHADERS_PATH "/screen.vert", pathToFragmentShader);
        std::cout << "Loaded shader from path " << pathToFragmentShader << std::endl;
}

ScreenFillingQuad::~ScreenFillingQuad()
{

}

void ScreenFillingQuad::drawQuad()
{
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

Shader ScreenFillingQuad::getShader()
{
	return *m_shader;
}

void ScreenFillingQuad::initializeQuad()
{
	if (!m_initialized) {
		float vertices[] = {
			-1.0f, -1.0f, 0.0, 0.0,
			-1.0f,  1.0f, 0.0, 1.0,
			1.0f, -1.0f, 1.0, 0.0,
			1.0f, -1.0f, 1.0, 0.0,
			-1.0f,  1.0f, 0.0, 1.0,
			1.0f,  1.0f, 1.0, 1.0
		};
		glGenVertexArrays(1, &m_quadVAO);
		glGenBuffers(1, &m_quadVBO);
		glBindVertexArray(m_quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
		m_initialized = true;
	}
}

