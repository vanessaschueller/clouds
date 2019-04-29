#pragma once

#include <Includes/Defs.h>

class ComputeShader
{
public:
	ComputeShader();
	~ComputeShader();

	void initFromFiles(std::string predefine, std::string computeShaderFileName);
	void initFromFiles(std::string computeShaderFileName);
	void initFromStrings(std::string computeShaderSource);

	void addStringDefinition(std::string definition);
	void use();
	void disable();

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

	GLuint computeShaderID;
	GLuint programID;
	
	std::string m_source;

	enum class ObjectType
	{
		SHADER, PROGRAM
	};
	bool initialised;
	int shaderCount;
	GLuint compileShader(std::string shaderSource, GLenum shaderType);
	void initialise(std::string computeShaderSource);
	std::string loadShaderFromFile(const std::string filename);
	std::string getInfoLog(ObjectType type, int id);
};