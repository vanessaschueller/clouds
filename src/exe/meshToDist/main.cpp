#include "Includes/Defs.h"
#include "Shader/ScreenFillingQuad.h"
#define WIDTH 900
#define HEIGHT 900

GLFWwindow* window;

int main() {
	/*
	 *Window definitions
	 */
	glfwInit();

	window = glfwCreateWindow(WIDTH, HEIGHT, "Realtime Clouds", nullptr, nullptr);
	glfwSetWindowPos(window, 100, 50);
	glfwMakeContextCurrent(window);

	glfwSwapInterval(0); // vsync
	glewInit();
	glClearColor(1, 1, 1, 0);
	glEnable(GL_DEPTH_TEST);
	ScreenFillingQuad quad(CLOUDS_SHADERS_PATH "/grid.frag");
	Shader quadShader = quad.getShader();

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0, 0, 0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		quadShader.use();
		quad.drawQuad();

		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
