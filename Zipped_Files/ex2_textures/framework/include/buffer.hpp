#pragma once

#include "gl.hpp"

namespace gpupro {

	// A buffer is a pure memory block on GPU side.
	class Buffer
	{
	public:
		// The type of a buffer determines its main purpose.
		// The buffer will be bound to that slot for several operations.
		// However, you can always bind the same buffer for different
		// purposes.
		enum class Type
		{
			VERTEX = GL_ARRAY_BUFFER,
			INDEX = GL_ELEMENT_ARRAY_BUFFER,
			SHADER_STORAGE = GL_SHADER_STORAGE_BUFFER,
			TEXTURE = GL_TEXTURE_BUFFER,
			UNIFORM = GL_UNIFORM_BUFFER,
			ATOMIC = GL_ATOMIC_COUNTER_BUFFER,
			INDIRECT_DISPATCH = GL_DISPATCH_INDIRECT_BUFFER,
			INDIRECT_DRAW = GL_DRAW_INDIRECT_BUFFER,
			TRANSFORM_FEEDBACK = GL_TRANSFORM_FEEDBACK_BUFFER,
		};

		enum Usage
		{
			// Allow glBufferSubData updates for this buffer.
			SUB_DATA_UPDATE = GL_DYNAMIC_STORAGE_BIT,
			// There will be more usages later...
		};

		// Create a raw buffer for data. Buffers created with this method
		// cannot be used as texture buffers!
		// _data: data for initialization.
		Buffer(Type _type, GLuint _elementSize, GLuint _numElements, Usage _usageBits = Usage(), const GLvoid* _data = nullptr);
		~Buffer();
		// Move but not copy-able
		Buffer(Buffer&& _rhs);
		Buffer(const Buffer&) = delete;
		Buffer& operator = (Buffer&& _rhs);
		Buffer& operator = (const Buffer& _rhs) = delete;

		// Bind as vertex buffer
		// _offset: offset to the first element in number of elements
		void bindAsVertexBuffer(GLuint _bindingIndex, GLuint _offset = 0);
		// Bind to GL_ELEMENT_ARRAY_BUFFER (index buffer)
		void bindAsIndexBuffer();
		// Bind as uniform buffer
		// _bindingIndex: binding slot of the uniform buffer
		// _offset: it is possible to bind a part of a larger buffer.
		//		This gives the offset in byte to the begin of the range.
		// _size: size of the range in bytes. -1 binds the entire buffer.
		void bindAsUniformBuffer(GLuint _bindingIndex, GLintptr _offset = 0, GLsizeiptr _size = GLsizeiptr(-1));

		// Upload a small chunk of data to a specific position.
		// Requires Usage::SUB_DATA_UPDATE.
		void subDataUpdate(GLintptr _offset, GLsizei _size, const GLvoid* _data);


		// Set the entire buffer content to zero.
		void clear();

		GLuint numElements() const { return m_size / m_elementSize; }

		GLuint glID() { return m_id; }
	private:
		GLuint m_id;
		Type m_type;
		GLsizei m_size;
		GLuint m_elementSize;
		Usage m_usage;
	};

} // namespace gpupro