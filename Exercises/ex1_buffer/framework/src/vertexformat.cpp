#include "vertexformat.hpp"

static bool isIntegerType(gpupro::VertexAttribute::Type _t)
{
	switch(_t)
	{
	case gpupro::VertexAttribute::Type::INT8:
	case gpupro::VertexAttribute::Type::UINT8:
	case gpupro::VertexAttribute::Type::INT16:
	case gpupro::VertexAttribute::Type::UINT16:
	case gpupro::VertexAttribute::Type::INT32:
	case gpupro::VertexAttribute::Type::UINT32:
		return true;
	default:
		return false;
	}
}

gpupro::VertexFormat::VertexFormat(const std::vector<VertexAttribute>& _attributes)
{
	// TODO: Generate the Vertex Array Object and bind it
	glGenVertexArrays(1, &m_id);
	glBindVertexArray(m_id);

	for(auto& attr : _attributes)
	{
		// TODO: Enable the attribute with index attr.attributIndex.
		glEnableVertexAttribArray(attr.attributIndex);


		// TODO: Setup all the attributes with glVertexAttribDivisor,
		// glVertexAttrib*Format and glVertexAttribBinding (last!).
		// Use glVertexAttribLFormat for DOUBLE, glVertexAttribIFormat for
		// all integer types (isIntegerType) and glVertexAttribFormat
		// otherwise.
		// For an unknown reason (driver bug?) this must be called AFTER glVertexAttrib*Format!
		// Otherwise it is overwritten and expects attribIndex == vboIndex.
		glVertexAttribDivisor(attr.attributIndex, attr.divisor);

		if (isIntegerType(attr.type)) {
			glVertexAttribIFormat(attr.attributIndex, attr.numComponents, attr.type, attr.offset);
		}
		else if (attr.type == GL_DOUBLE){
			glVertexAttribLFormat(attr.attributIndex, attr.numComponents, attr.type, attr.offset);
		}
		else {
			glVertexAttribFormat(attr.attributIndex, attr.numComponents, attr.type, attr.normalized, attr.offset);
		}

		glVertexAttribBinding(attr.attributIndex, attr.vboBindingIndex);
	}
}

gpupro::VertexFormat::~VertexFormat()
{
	// TODO: Delete the Vertex Array Object.
	glDeleteVertexArrays(1, &m_id);
}

gpupro::VertexFormat::VertexFormat(VertexFormat&& _rhs) :
	m_id(_rhs.m_id)
{
	_rhs.m_id = 0;
}

gpupro::VertexFormat& gpupro::VertexFormat::operator=(VertexFormat&& _rhs)
{
	// TODO: Delete the Vertex Array Object.
	glDeleteVertexArrays(1, &m_id);

	m_id = _rhs.m_id;
	_rhs.m_id = 0;

	return *this;
}
