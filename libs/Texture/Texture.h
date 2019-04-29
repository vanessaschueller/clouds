#pragma once 
#include <Includes/Defs.h>

class Texture
{
public:
	Texture();
	Texture(int w, int h);
	Texture(GLuint internalFormat, GLuint format, GLuint type, GLuint filter);
	Texture(GLuint internalFormat, GLuint format, GLuint type);

	void init(GLuint internalFormat, GLuint format, GLuint type);
	/*
	* Load texture from image
	*/
	Texture(std::string pathToFile, int dim, int channel);
	~Texture();
	/*
	* Load texture from image
	*/
	bool load(std::string pathToFile, int dim, int channel);
	GLuint getTextureID();
	void clear();

    //GLuint genTexture(int w, int h);
    GLuint gen2DTexture(int w, int h);
    //GLuint gen2DTextureArray(int w, int h, int z);
    //GLuint gen2DTextureNoise(int w, int h);
    GLuint gen2DTexture(int w, int h, float initVal);
    //GLuint genUimageBuffer(int size);
    GLuint gen3DTexture(int x, int y, int z);
private:
	void createTexture(int dim, int channel);


	unsigned char *m_data;
	GLuint m_textureID;
	int m_width, m_height, m_bytesPerPixel;
	unsigned char* pixels;
	GLuint m_target;
	GLuint m_type;
	GLuint m_internalFormat;
	GLuint m_format;
	GLuint m_filter;
	bool m_isImageTex;
	bool m_isLayered;

	int m_x, m_y, m_z;
    
};
