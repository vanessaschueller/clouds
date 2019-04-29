#include <Texture/Texture.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

Texture::Texture()
{
	m_bytesPerPixel = 0;
	m_width = 0;
	m_height = 0;
	m_data = nullptr;
	pixels = nullptr;
	m_target = 0;
	m_type = 0;
	m_internalFormat = 0;
	m_format = 0;
	m_filter = 0;
	m_isImageTex = false;
	m_isLayered = false;
	m_x = 0;
	m_y = 0;
	m_z = 0;
}

Texture::Texture(GLuint internalFormat, GLuint format, GLuint type, GLuint filter)
{
	m_bytesPerPixel = 0;
	m_width = 0;
	m_height = 0;
	m_data = nullptr;
	pixels = nullptr;
	m_target = 0;
	m_isLayered = false;
	m_x = 0;
	m_y = 0;
	m_z = 0;
	m_type = type;
	m_internalFormat = internalFormat;
	m_format = format;
	m_filter = filter;
	m_isImageTex = false;
}

Texture::Texture(GLuint internalFormat, GLuint format, GLuint type)
{
	m_bytesPerPixel = 0;
	m_width = 0;
	m_height = 0;
	m_data = nullptr;
	pixels = nullptr;
	m_target = 0;
	m_isLayered = false;
	m_x = 0;
	m_y = 0;
	m_z = 0;
	m_type = type;
	m_internalFormat = internalFormat;
	m_format = format;
    m_filter         = GL_LINEAR;
	m_isImageTex = false;
}

void Texture::init(GLuint internalFormat, GLuint format, GLuint type) {
    m_bytesPerPixel  = 0;
    m_width          = 0;
    m_height         = 0;
    m_data           = nullptr;
    pixels           = nullptr;
    m_target         = 0;
    m_isLayered      = false;
    m_x              = 0;
    m_y              = 0;
    m_z              = 0;
    m_type           = type;
    m_internalFormat = internalFormat;
    m_format         = format;
    m_filter         = GL_LINEAR;
    m_isImageTex     = false;
}

Texture::Texture(std::string pathToFile, int dim, int channel)
{
	m_bytesPerPixel = 0;
	m_width = 0;
	m_height = 0;
	m_data = nullptr;
	pixels = nullptr;
	m_target = 0;
	m_type = 0;
	m_internalFormat = 0;
	m_format = 0;
	m_filter = 0;
	m_isImageTex = false;
	m_isLayered = false;
	m_x = 0;
	m_y = 0;
	m_z = 0;
	load(pathToFile, dim, channel);
}

Texture::~Texture()
{
}

GLuint Texture::getTextureID()
{
	return m_textureID;
}

bool Texture::load(std::string pathToFile, int dim, int channel)
{
	if (channel < 4)
	{
		m_data = stbi_load(pathToFile.c_str(), &m_width, &m_height, &m_bytesPerPixel, 0);
	}
	if (channel == 4)
	{
		m_data = stbi_load(pathToFile.c_str(), &m_width, &m_height, &m_bytesPerPixel, 4);
	}
	if (dim != 2 && dim != 3)
	{
		std::cout << "FAIL: Could not generate texture for " << pathToFile.c_str() << std::endl;
		return false;
	}

	createTexture(dim, channel);
	   
	std::cout << "SUCCESS: Loaded texture image " << pathToFile.c_str() << std::endl;
	return true;
}

void Texture::createTexture(int dim, int channel)
{
	if (dim == 2)
	{
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(m_data);
	}
	if (dim == 3 && channel == 3)
	{
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_3D, m_textureID);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, m_height, m_height, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_data);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D, 0);
		stbi_image_free(m_data);
	}
	if (dim == 3 && channel == 4)
	{
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_3D, m_textureID);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, m_height, m_height, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_data);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D, 0);
		stbi_image_free(m_data);
	}
}

void Texture::clear()
{
	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, pixels);
	glBindImageTexture(0, m_textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

}

GLuint Texture::gen2DTexture(int w, int h)
{
	return gen2DTexture(w, h, 0);
}

GLuint Texture::gen2DTexture(int w, int h, float initVal)
{
	glEnable(GL_TEXTURE_2D);
	m_target = GL_TEXTURE_2D;
	m_isImageTex = true;
	m_isLayered = GL_FALSE;

	// byteCount = sizeof(unsigned int) * w*h;
	// pixels = new unsigned char[byteCount];

	float f = initVal;
	unsigned char const * p = reinterpret_cast<unsigned char const *>(&f);
	pixels = new unsigned char[4]{ p[0],p[1],p[2],p[3] };


	GLuint t;
	glGenTextures(1, &t);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(m_target, t);
	glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_filter);
	glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, m_filter);
        glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(m_target, 0, m_internalFormat, w, h, 0, m_format, m_type, 0);
	glBindImageTexture(0, t, 0, GL_FALSE, 0, GL_READ_WRITE, m_internalFormat);
	glClearTexImage(t, 0, m_format, m_type, pixels);

	m_textureID = t;

	return t;
}

GLuint Texture::gen3DTexture(int x, int y, int z)
{
	glEnable(GL_TEXTURE_3D);
	m_target = GL_TEXTURE_3D;
	m_isImageTex = true;
	m_isLayered = GL_TRUE;
	m_x = x;
	m_y = y;
	m_z = z;

	m_bytesPerPixel = sizeof(unsigned int) * x*y*z;
	pixels = new unsigned char[m_bytesPerPixel];

	float f = 0;

	unsigned char const * p = reinterpret_cast<unsigned char const *>(&f);

	//    for (int i = 0; i < byteCount; i+=4)
	//    {
	//        pixels[i] = p[0];
	//        pixels[i+1] = p[1];
	//        pixels[i+2] = p[2];
	//        pixels[i+3] = p[3];
	//    }
	pixels = new unsigned char[4]{ p[0],p[1],p[2],p[3] };

	GLuint t;
	glGenTextures(1, &t);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(m_target, t);
	glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_filter);
	glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, m_filter);
        glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage3D(m_target, 0, m_internalFormat, x, y, z, 0, m_format, m_type, 0);
	glBindImageTexture(0, t, 0, GL_TRUE, 0, GL_READ_WRITE, m_internalFormat);
	glClearTexImage(t, 0, m_format, m_type, pixels);

	m_textureID = t;

	return t;
}

