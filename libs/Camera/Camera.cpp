#include "Camera.h"

Camera::Camera() {}

Camera::Camera(GLFWwindow* window, int width, int height, glm::mat4 projection)
{
	m_window = window;
	m_camPos = glm::vec3(0.0f, 0.0f, 5.0f);
	m_direction = glm::vec3(0.0f, 0.0f, -1.0f);
	m_up = glm::vec3(0.0f, 1.0f, 0.0f);

	m_width = width;
	m_height = height;

	m_speed = 0.0f;
	m_speedImgPlane = 0.0f;
	m_theta = glm::pi<float>() / 2.0f;
	m_phi = glm::pi<float>();

	m_locX = 0;
	m_locY = 0;

	m_viewMatrix = glm::lookAt(m_camPos, m_camPos + m_direction, m_up);

	m_oldX = width / 2.f;
	m_oldY = height / 2.f;
	m_fov = 60.f;

	m_projectionMatrix = projection;
}

Camera::Camera(GLFWwindow * window, int width, int height)
{
	m_window = window;
	m_camPos = glm::vec3(0.0f, 0.0f, 5.0f);
	m_direction = glm::vec3(0.0f, 0.0f, -1.0f);
	m_up = glm::vec3(0.0f, 1.0f, 0.0f);

	m_width = width;
	m_height = height;

	m_speed = 0.0f;
	m_speedImgPlane = 0.0f;
	m_theta = glm::pi<float>() / 2.0f;
	m_phi = glm::pi<float>();

	m_locX = 0;
	m_locY = 0;

	m_viewMatrix = glm::lookAt(m_camPos, m_camPos + m_direction, m_up);

	m_oldX = width / 2.f;
	m_oldY = height / 2.f;
	m_fov = 60.f;
	m_projectionMatrix = glm::perspective(glm::radians(m_fov), float(m_width)/m_height, .1f, 100000.f);
}

Camera::~Camera()
{
}

void Camera::setViewport(int loc_x, int loc_y, int width, int height) {

	m_locX = loc_x;
	m_locY = loc_y;
	m_width = width;
	m_height = height;
}

void Camera::setFOV(float fov)
{
	m_fov = fov;
}

void Camera::update(double deltaTime) 
{
	glViewport(m_locX, m_locY, m_width, m_height);
	double x, y;
	glfwGetCursorPos(m_window, &x, &y);
	y = m_height - y;
	if (ImGui::GetIO().WantCaptureMouse) {
		return;
	}
	else {
		if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			float changeX = ((float)x - m_oldX) * SENSITIVITY;
			float changeY = ((float)y - m_oldY) * SENSITIVITY;

			m_theta -= changeY;
			if (m_theta < 0.01f) m_theta = 0.01f;
			else if (m_theta > glm::pi<float>() - 0.01f) m_theta = glm::pi<float>() - 0.01f;

			m_phi -= changeX;
			if (m_phi < 0) m_phi += 2 * glm::pi<float>();
			else if (m_phi > 2 * glm::pi<float>()) m_phi -= 2 * glm::pi<float>();

			m_oldX = (float)x;
			m_oldY = (float)y;

			m_direction.x = sin(m_theta) * sin(m_phi);
			m_direction.y = cos(m_theta);
			m_direction.z = sin(m_theta) * cos(m_phi);
		}
		else
		{
			m_oldX = (float)x;
			m_oldY = (float)y;
		}

		if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
			m_camPos += glm::mat3(glm::rotate(glm::mat4(1.0f), m_phi, glm::vec3(0, 1.f, 0)))* glm::vec3(0, 0, 500.0f) * (float)deltaTime;
		if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
			m_camPos -= glm::mat3(glm::rotate(glm::mat4(1.0f), m_phi, glm::vec3(0, 1.f, 0)))* glm::vec3(0, 0, 500.0f) * (float)deltaTime;
		if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
			m_camPos += glm::mat3(glm::rotate(glm::mat4(1.0f), m_phi, glm::vec3(0, 1.f, 0)))* glm::vec3(500.0f, 0, 0) * (float)deltaTime;
		if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			m_camPos -= glm::mat3(glm::rotate(glm::mat4(1.0f), m_phi, glm::vec3(0, 1.f, 0)))* glm::vec3(500.0f, 0, 0) * (float)deltaTime;

		else
			m_speed = (m_speed > 0) ?
			m_speed - 5.0f * (float)deltaTime : m_speed + 5.0f * (float)deltaTime;

		m_camPos += m_speed * m_direction * (float)deltaTime;
		m_viewMatrix = glm::lookAt(m_camPos, m_camPos + m_direction, m_up);
		m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_ratio, .1f, 100000.f);
	}
}

void Camera::setCamPos(glm::vec3 * campos)
{
	m_camPos = *campos;
}

glm::vec3 Camera::getCamPos()
{
	return m_camPos;
}

float Camera::getFOV()
{
	return m_fov;
}

void Camera::setLookAt(glm::vec3 * position, glm::vec3 * center)
{
	m_camPos = *position;
	glm::vec3 diff = glm::normalize(*center - *position);
	m_direction = diff;

	m_theta = acos(diff.y);
	if (m_theta < 0.01f) m_theta = 0.01f;
	m_phi = acos(diff.z / (sin(m_theta) + 0.0001f));
	if (diff.x > 0) m_phi = 2 * glm::pi<float>() - m_phi;
}


void Camera::setUpvector(glm::vec3 * up)
{
	m_up = *up;
}

void Camera::setWidthHeight(int w, int h)
{
	m_width = w;
	m_height = h;
	m_ratio = (float)m_width / m_height;
}

glm::mat4 Camera::getView()
{
	return m_viewMatrix;
}

glm::mat4 Camera::getProj()
{
	return m_projectionMatrix; }

float Camera::getSpeed() { return m_speed; }

float Camera::getTheta() { return m_theta; }

float Camera::getPhi() { return m_phi; }

glm::vec3 Camera::getDirection() { return m_direction; }

glm::vec3 Camera::getUp() { return m_up; }

float Camera::getRatio() { return m_ratio; }

