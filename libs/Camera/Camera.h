#pragma once

#include <Includes/Defs.h>

class Camera
{
public:
    Camera();

	Camera(GLFWwindow* window, int width, int height, glm::mat4 projection);

	Camera(GLFWwindow* window, int width, int height);

	~Camera();

	void update(double deltaTime);

	void setCamPos(glm::vec3 *campos);

	glm::vec3 getCamPos();

	float getFOV();

	void setLookAt(glm::vec3 *position, glm::vec3 *center);

	void setUpvector(glm::vec3 *up);

	void setWidthHeight(int w, int h);

	void setViewport(int loc_x, int loc_y, int width, int height);

	void setFOV(float fov);

	glm::mat4 getView();

	glm::mat4 getProj();
        float     getSpeed();
        float     getTheta();
        float     getPhi();
        glm::vec3 getDirection();
        glm::vec3 getUp();
        float     getRatio();

private:
	constexpr static float SENSITIVITY = 0.005f; //<!< the sensitivity of the movement

	float m_speed, m_speedImgPlane;
	float m_theta, m_phi;

	glm::vec3 m_camPos, m_direction, m_up;

	float m_ratio;
	int m_width, m_height;
	glm::mat4 m_viewMatrix, m_projectionMatrix;

	int m_locX, m_locY;
	float m_fov;
	double m_oldX, m_oldY;

	GLFWwindow *m_window;

};

