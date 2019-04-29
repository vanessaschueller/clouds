#pragma once

#include<Includes/Defs.h>
#include <map>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

class Shader
{
public:

	Shader();
	~Shader();

	void initFromFiles(std::string shaderFileName);
	void initFromFiles(std::string vertexShaderFileName, std::string fragmentShaderFileName);
	void initFromFiles(std::string vertexShaderFileName, std::string geomShaderFileName, std::string fragmentShaderFileName);
	void initFromStrings(std::string vertexShaderSource, std::string fragmentShaderSource);
	
	void use();
	void disable();
	GLuint getID();
	void setBool(const std::string &name, bool value) const;
	void setUint(const std::string &name, unsigned int value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void setuiVec3(const std::string &name, glm::uvec3 vector) const;
	void setVec2(const std::string &name, glm::vec2 vector) const;
	void setVec3(const std::string &name, glm::vec3 vector) const;
	void setVec4(const std::string &name, glm::vec4 vector) const;
	void setMat4(const std::string &name, glm::mat4 matrix) const;
	void setSampler2D(const std::string &name, unsigned int texture, int id) const;
	void setSampler3D(const std::string &name, unsigned int texture, int id) const;

private:
	static const bool DEBUG = true;

	enum class ObjectType
	{
		SHADER, PROGRAM
	};

	GLuint vertexShaderID;
	GLuint geomShaderID;
	GLuint fragmentShaderID;
	GLuint programID;
	GLuint shaderCount;

	std::map<std::string, int> attributeMap;
	std::map<std::string, int> uniformMap;

	bool initialised;

	GLuint compileShader(std::string shaderSource, GLenum shaderType);
	void initialise(std::string shaderSource);
	void initialise(std::string vertexShaderSource, std::string fragmentShaderSource);
	void initialise(std::string vertexShaderSource, std::string geomShaderSource, std::string fragmentShaderSource);
	std::string loadShaderFromFile(const std::string filename);
	std::string getInfoLog(ObjectType type, int id);
};


