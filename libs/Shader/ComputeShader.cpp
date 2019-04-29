#include <Shader/ComputeShader.h>

ComputeShader::ComputeShader() {
  initialised = false;
  programID = glCreateProgram();
  glUseProgram(programID);
  shaderCount = 0;
}

ComputeShader::~ComputeShader() { glDeleteProgram(programID); }

void ComputeShader::initFromFiles(std::string predefine, std::string computeShaderFileName)
{
	m_source = predefine + loadShaderFromFile(computeShaderFileName);
	initialise(m_source);
}

void ComputeShader::initFromFiles(std::string computeShaderFileName) {
  m_source = loadShaderFromFile(computeShaderFileName);
  initialise(m_source);
}

void ComputeShader::initFromStrings(std::string computeShaderSource) {
  initialise(computeShaderSource);
}

void ComputeShader::addStringDefinition(std::string definition)
{
	m_source = definition + m_source;
}

void ComputeShader::use() {
  if (initialised) {
    glUseProgram(programID);
  } else {
    std::string msg = "Shader program " + programID;
    msg += " not initialised - aborting.";
    throw std::runtime_error(msg);
  }
}

void ComputeShader::disable() { glUseProgram(0); }

void ComputeShader::setBool(const std::string &name, bool value) const {
  glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value);
}

void ComputeShader::setUint(const std::string & name, unsigned int value) const
{
	glUniform1ui(glGetUniformLocation(programID, name.c_str()), value);

}

void ComputeShader::setInt(const std::string &name, int value) const {
  glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

void ComputeShader::setFloat(const std::string &name, float value) const {
  glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

void ComputeShader::setuiVec3(const std::string & name, glm::uvec3 vector) const
{
	unsigned int location = glGetUniformLocation(programID, name.c_str());

	glUniform3uiv(location, 1, glm::value_ptr(vector));
}

void ComputeShader::setVec2(const std::string &name, glm::vec2 vector) const {
  unsigned int location = glGetUniformLocation(programID, name.c_str());

  glUniform2fv(location, 1, glm::value_ptr(vector));
}

void ComputeShader::setVec3(const std::string &name, glm::vec3 vector) const {
  unsigned int location = glGetUniformLocation(programID, name.c_str());

  glUniform3fv(location, 1, glm::value_ptr(vector));
}

void ComputeShader::setVec4(const std::string &name, glm::vec4 vector) const {
  unsigned int location = glGetUniformLocation(programID, name.c_str());

  glUniform4fv(location, 1, glm::value_ptr(vector));
}

void ComputeShader::setMat4(const std::string &name, glm::mat4 matrix) const {
  unsigned int mat = glGetUniformLocation(programID, name.c_str());

  glUniformMatrix4fv(mat, 1, false, glm::value_ptr(matrix));
}

void ComputeShader::setSampler2D(const std::string &name, unsigned int texture, int id) const
{
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, texture);
	setInt(name, id);
}

void ComputeShader::setSampler3D(const std::string &name, unsigned int texture, int id) const
{
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_3D, texture);
	setInt(name, id);
}

GLuint ComputeShader::compileShader(std::string shaderSource,
                                    GLenum shaderType) {
  std::string shaderTypeString;
  switch (shaderType) {
  case GL_COMPUTE_SHADER:
    shaderTypeString = "GL_COMPUTE_SHADER";
    break;

  default:
    throw std::runtime_error("Bad shader type enum in compileShader.");
    break;
  }

  // Generate a shader id
  // Note: Shader id will be non-zero if successfully created.
  GLuint shaderId = glCreateShader(shaderType);
  if (shaderId == 0) {
    // Display the shader log via a runtime_error
    throw std::runtime_error("Could not create shader of type " +
                             shaderTypeString + ": " +
                             getInfoLog(ObjectType::SHADER, shaderId));
  }

  // Get the source string as a pointer to an array of characters
  const char *shaderSourceChars = shaderSource.c_str();

  // Attach the GLSL source code to the shader
  // Params: GLuint shader, GLsizei count, const GLchar **string, const GLint
  // *length Note: The pointer to an array of source chars will be null
  // terminated, so we don't need to specify the length and can instead use
  // NULL.
  glShaderSource(shaderId, 1, &shaderSourceChars, NULL);

  // Compile the shader
  glCompileShader(shaderId);

  // Check the compilation status and throw a runtime_error if shader
  // compilation failed
  GLint shaderStatus;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &shaderStatus);
  if (shaderStatus == GL_FALSE) {
    GLint infoLogLength;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);

    GLchar *infoLog = new GLchar[infoLogLength + 1];
    glGetShaderInfoLog(shaderId, infoLogLength, nullptr, infoLog);

    std::cout << "ERROR: Unable to compile shader" << std::endl
              << infoLog << std::endl;
    delete[] infoLog;
  } else {
    std::cout << "SUCCESS: Shader compiled" << std::endl;
  }

  // If everything went well, return the shader id
  return shaderId;
}

void ComputeShader::initialise(std::string computeShaderSource)
{
	// Compile the shaders and return their id values
	computeShaderID = compileShader(computeShaderSource, GL_COMPUTE_SHADER);

	// Attach the compiled shaders to the shader program
	glAttachShader(programID, computeShaderID);

	// Link the shader program - details are placed in the program info log
	glLinkProgram(programID);

	// Once the shader program has the shaders attached and linked, the shaders are no longer required.
	// If the linking failed, then we're going to abort anyway so we still detach the shaders.
	glDetachShader(programID, computeShaderID);

	// Check the program link status and throw a runtime_error if program linkage failed.
	GLint programLinkSuccess = GL_FALSE;
	glGetProgramiv(programID, GL_LINK_STATUS, &programLinkSuccess);
	if (programLinkSuccess == GL_TRUE)
	{
		if (DEBUG)
		{
			std::cout << "Shader program link successful." << std::endl;
		}
	}
	else
	{
		throw std::runtime_error("Shader program link failed: " + getInfoLog(ObjectType::PROGRAM, programID));
	}

	//// Validate the shader program
	//glValidateProgram(programID);

	//// Check the validation status and throw a runtime_error if program validation failed
	//GLint programValidatationStatus;
	//glGetProgramiv(programID, GL_VALIDATE_STATUS, &programValidatationStatus);
	//if (programValidatationStatus == GL_TRUE)
	//{
	//	if (DEBUG)
	//	{
	//		std::cout << "Shader program validation successful." << std::endl;
	//	}
	//}
	//else
	//{
	//	throw std::runtime_error("Shader program validation failed: " + getInfoLog(ObjectType::PROGRAM, programID));
	//}

	// Finally, the shader program is initialised
	initialised = true;
}

std::string ComputeShader::loadShaderFromFile(const std::string filename)
{
	// Create an input filestream and attempt to open the specified file
	std::ifstream file(filename.c_str());

	// If we couldn't open the file we'll bail out
	if (!file.good())
	{
		throw std::runtime_error("Failed to open file: " + filename);
	}

	// Otherwise, create a string stream...
	std::stringstream stream;

	// ...and dump the contents of the file into it.
	stream << file.rdbuf();

	// Now that we've read the file we can close it
	file.close();

	// Finally, convert the stringstream into a string and return it
	return stream.str();
}

std::string ComputeShader::getInfoLog(ObjectType type, int id)
{
	GLint infoLogLength;
	if (type == ObjectType::SHADER)
	{
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
	}
	else // type must be ObjectType::PROGRAM
	{
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
	}

	GLchar *infoLog = new GLchar[infoLogLength + 1];
	if (type == ObjectType::SHADER)
	{
		glGetShaderInfoLog(id, infoLogLength, NULL, infoLog);
	}
	else // type must be ObjectType::PROGRAM
	{
		glGetProgramInfoLog(id, infoLogLength, NULL, infoLog);
	}

	// Convert the info log to a string
	std::string infoLogString(infoLog);

	// Delete the char array version of the log
	delete[] infoLog;

	// Finally, return the string version of the info log
	return infoLogString;
}
