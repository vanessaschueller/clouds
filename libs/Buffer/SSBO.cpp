#include "SSBO.h"

SSBO::SSBO()
{
	m_currentSize = 0;
	m_bufferObject = INVALID_GL_VALUE;
	m_name = "noName";
}

SSBO::~SSBO()
{
	if (m_bufferObject != INVALID_GL_VALUE)
		release();
}

void SSBO::setName(std::string name)
{
	m_name = name;
}

void SSBO::release()
{
	glDeleteBuffers(1, &m_bufferObject);
	m_bufferObject = INVALID_GL_VALUE;
	m_currentSize = 0;
}

bool SSBO::create(GLsizeiptr bytes)
{
	// create the buffers and bind the data 
	if (m_bufferObject == INVALID_GL_VALUE)
		glCreateBuffers(1, &m_bufferObject);

	return allocate(bytes);
}

bool SSBO::allocate(GLsizeiptr bytes)
{
	//getStatus();
	GLenum error = glGetError();

	glNamedBufferStorage(m_bufferObject, bytes, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	error = glGetError();
	//getStatus();

	if (error == GL_OUT_OF_MEMORY)
	{
		printf("\nERROR: GPU Out of memory for SSBO.\n");
		release();
		return false;
	}
	else if (error == GL_INVALID_ENUM)
	{
		printf("ERROR: GL_INVALID_ENUM for glNamedBufferStorage in SSBO %s\n", m_name.c_str());
		printf("%d, %tu, %d, %u\n", m_bufferObject, bytes, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
		return false;
	}
	else if (error == GL_INVALID_OPERATION)
	{
		printf("ERROR: GL_INVALID_OPERATION for glNamedBufferStorage in SSBO %s\n", m_name.c_str());
		printf("%d, %td, %d, %d\n", m_bufferObject, bytes, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
		return false;
	}
	else if (error == GL_INVALID_VALUE)
	{
		printf("ERROR: GL_INVALID_VALUE for glNamedBufferStorage in SSBO %s\n", m_name.c_str());
		printf("%d, %td, %d, %d\n", m_bufferObject, bytes, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
		return false;
	}
	{
		//printf("%d, %d, %d, %d\n", m_bufferObject, bytes, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
		m_currentSize = bytes;
		//printInfo();
		return true;
	}
}

bool SSBO::resize(GLsizeiptr bytes)
{
	// deletes old values!!
	if (bytes == m_currentSize)
		return true;

	if (m_currentSize == 0)
		return create(bytes); //if no data is allocated, create the buffer (calls allocate)
	else
	{
		release(); //resize for storage buffer is not possible (???)
		return create(bytes); //it has to be released and a new allocation is necessary
	}
	return false;
}

unsigned int SSBO::getSize()
{
	return m_currentSize;
}

GLuint SSBO::getID()
{
	return m_bufferObject;
}

void *SSBO::map(GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	void* ptr;
	ptr = glMapNamedBufferRange(m_bufferObject, offset, length, access);
	return ptr;
}

void SSBO::unmap()
{
	glUnmapNamedBuffer(m_bufferObject);
}

void SSBO::clear()
{
	unsigned int clear_val = 0;
	glClearNamedBufferData(m_bufferObject, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, &clear_val);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void SSBO::copy(SSBO *dest, unsigned int offsetFrom, unsigned int offsetTo, unsigned int size)
{
	//untested!
	assert(m_currentSize >= offsetFrom + size);
	assert(dest->m_currentSize >= offsetTo + size);
	glCopyNamedBufferSubData(m_bufferObject, dest->getID(), offsetFrom, offsetTo, size);
}

void SSBO::print(unsigned int offset, unsigned int lenght, unsigned int itemsPerLine)
{
	unsigned int *p = (unsigned int*)map(offset, lenght, GL_MAP_READ_BIT);
	for (int i = offset; i < offset + lenght; i++)
	{
		printf("%d: %d, ", i, p[i]);
		if ((i % itemsPerLine) == 0)
			printf("\n");
	}
	printf("\n");
	unmap();
}

void SSBO::printInfo()
{
	std::cout << "INFO: SSBO " << m_name << ": " << m_currentSize << " bytes" << '\n';
}

void SSBO::getStatus()
{
	GLint params;
	GLint64 params64;
	printf("  %s\n", m_name.c_str());
	glGetNamedBufferParameteriv(m_bufferObject, GL_BUFFER_ACCESS, &params);
	printf("GL_BUFFER_ACCESS %d\n", params);
	glGetNamedBufferParameteriv(m_bufferObject, GL_BUFFER_ACCESS_FLAGS, &params);
	printf("GL_BUFFER_ACCESS_FLAGS %d\n", params);
	glGetNamedBufferParameteriv(m_bufferObject, GL_BUFFER_IMMUTABLE_STORAGE, &params);
	printf("GL_BUFFER_IMMUTABLE_STORAGE %d\n", params);
	glGetNamedBufferParameteriv(m_bufferObject, GL_BUFFER_MAPPED, &params);
	printf("GL_BUFFER_MAPPED %d\n", params);
	glGetNamedBufferParameteri64v(m_bufferObject, GL_BUFFER_MAP_LENGTH, &params64);
	printf("GL_BUFFER_MAP_LENGTH %I64d\n", params64);
	glGetNamedBufferParameteri64v(m_bufferObject, GL_BUFFER_MAP_OFFSET, &params64);
	printf("GL_BUFFER_MAP_OFFSET %I64d\n", params64);
	glGetNamedBufferParameteriv(m_bufferObject, GL_BUFFER_SIZE, &params);
	printf("GL_BUFFER_SIZE %d\n", params);
	glGetNamedBufferParameteriv(m_bufferObject, GL_BUFFER_STORAGE_FLAGS, &params);
	printf("GL_BUFFER_STORAGE_FLAGS %d\n", params);
	glGetNamedBufferParameteriv(m_bufferObject, GL_BUFFER_USAGE, &params);
	printf("GL_BUFFER_USAGE %d\n", params);
}
