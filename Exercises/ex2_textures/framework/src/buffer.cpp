#include "buffer.hpp"
#include <iostream>

gpupro::Buffer::Buffer(Type _type, GLuint _elementSize, GLuint _numElements, Usage _usageBits, const GLvoid* _data) :
	m_type(_type),
	m_size(_elementSize * _numElements),
	m_elementSize(_elementSize),
	m_usage(_usageBits)
{
	// Generated one buffer
	glGenBuffers(1, &m_id);
	// Allocate space
	glBindBuffer(static_cast<GLenum>(m_type), m_id);
	glBufferStorage(static_cast<GLenum>(m_type), m_size, _data, static_cast<GLbitfield>(_usageBits));
}

gpupro::Buffer::~Buffer()
{
	glDeleteBuffers(1, &m_id);
}

gpupro::Buffer::Buffer(Buffer&& _rhs) :
	m_id(_rhs.m_id),
	m_type(_rhs.m_type),
	m_size(_rhs.m_size),
	m_elementSize(_rhs.m_elementSize),
	m_usage(_rhs.m_usage)
{
	_rhs.m_id = 0;
}

gpupro::Buffer& gpupro::Buffer::operator=(Buffer&& _rhs)
{
	glDeleteBuffers(1, &m_id);

	m_id = _rhs.m_id;
	m_type = _rhs.m_type;
	m_size = _rhs.m_size;
	m_elementSize = _rhs.m_elementSize;
	m_usage = _rhs.m_usage;
	_rhs.m_id = 0;
	return *this;
}

void gpupro::Buffer::bindAsVertexBuffer(GLuint _bindingIndex, GLuint _offset)
{
	glBindVertexBuffer(_bindingIndex, m_id, _offset * m_elementSize, m_elementSize);
}

void gpupro::Buffer::bindAsIndexBuffer()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

void gpupro::Buffer::bindAsUniformBuffer(GLuint _bindingIndex, GLintptr _offset, GLsizeiptr _size)
{
	if(_size == -1)
		_size = m_size - _offset;
	glBindBufferRange(GL_UNIFORM_BUFFER, _bindingIndex, m_id, _offset, _size);
}

void gpupro::Buffer::subDataUpdate(GLintptr _offset, GLsizei _size, const GLvoid* _data)
{
	if(!(m_usage & Usage::SUB_DATA_UPDATE)) {
		std::cerr << "ERR: Buffer::subDataUpdate requires Usage::SUB_DATA_UPDATE. The current buffer is static.\n";
		return;
	}

	if(_size == -1)
		_size = m_size - (GLsizei)_offset;

	// Bind to arbitrary buffer point to call the storage command
	glBindBuffer(static_cast<GLenum>(m_type), m_id);
	glBufferSubData(static_cast<GLenum>(m_type), _offset, _size, _data);
}

void gpupro::Buffer::clear()
{
	glBindBuffer(static_cast<GLenum>(m_type), m_id);
	unsigned zero = 0;
	glClearBufferData(static_cast<GLenum>(m_type), GL_R32UI, GL_RED, GL_UNSIGNED_INT, &zero);
}