#pragma once

#include <Includes/Defs.h>

class SSBO {
public:
	SSBO();
	~SSBO();

	void setName(std::string name);
	bool resize(GLsizeiptr bytes);
	unsigned int getSize();

	GLuint getID();

	void *map(GLintptr offset, GLsizeiptr length, GLbitfield access);
	void unmap();
	void copy(SSBO *dest, unsigned int offsetFrom, unsigned int offsetTo, unsigned int size);
	void clear();
	void print(unsigned int offset, unsigned int length, unsigned int itemsPerLine);
	void getStatus();
	void printInfo();

private:

	GLsizeiptr m_currentSize;
	GLuint m_bufferObject;
	std::string m_name;

	void release();
	bool create(GLsizeiptr bytes);
	bool allocate(GLsizeiptr bytes);

};
