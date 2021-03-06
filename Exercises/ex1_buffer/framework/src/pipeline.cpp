#include "pipeline.hpp"

gpupro::SamplerState::SamplerState(
	Filter _minFilter,
	Filter _magFilter,
	Filter _mipFilter,
	float _maxAnisotropy,
	DepthCompareFunc _depthCmpFunc,
	BorderHandling _borderHandling,
	float _borderColor[4]
)
#ifdef DEBUG
	: m_minFilter(_minFilter),
	m_magFilter(_magFilter),
	m_mipFilter(_mipFilter),
	m_depthCmpFunc(_depthCmpFunc),
	m_borderHandling(_borderHandling)
#endif
{
	// TODO: Generate a new sampler object in m_id.
	glGenSamplers(1, &m_id);

	// For all following TODOs you need the glSamplerParameteri command.
	// In most cases you only need the parameter name and can cast the
	// parameter value from the input

	// TODO: Set border handling (Parameter: GL_TEXTURE_WRAP_R...).
	// Use the same behavior for each dimension.

	// TODO: Set min and mag filter. The mipmap is part of the min-filter.
	// I.e. _minFilter and _mipFilter inputs must be combined to a single
	// value. For example GL_NEAREST_MIPMAP_NEAREST.

	if(_maxAnisotropy > 1.0f)
		// TODO: Set the max-anisotropy (GL_TEXTURE_MAX_ANISOTROPY_EXT)

	if(_depthCmpFunc != DepthCompareFunc::DISABLE)
	{
		glSamplerParameteri(m_id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glSamplerParameteri(m_id, GL_TEXTURE_COMPARE_FUNC, static_cast<GLint>(_depthCmpFunc));
	}

	if(_borderColor)
		glSamplerParameterfv(m_id, GL_TEXTURE_BORDER_COLOR, _borderColor);
}

gpupro::SamplerState::~SamplerState()
{
	glDeleteSamplers(1, &m_id);
}

gpupro::SamplerState::SamplerState(SamplerState&& _rhs) :
	m_id(_rhs.m_id)
#ifdef DEBUG
	, m_minFilter(_rhs.m_minFilter),
	m_magFilter(_rhs.m_magFilter),
	m_mipFilter(_rhs.m_mipFilter),
	m_depthCmpFunc(_rhs.m_depthCmpFunc),
	m_borderHandling(_rhs.m_borderHandling)
#endif
{
	_rhs.m_id = 0;
}

gpupro::SamplerState& gpupro::SamplerState::operator=(SamplerState&& _rhs)
{
	glDeleteSamplers(1, &m_id);

	m_id = _rhs.m_id;
#ifdef DEBUG
	m_minFilter = _rhs.m_minFilter;
	m_magFilter = _rhs.m_magFilter;
	m_mipFilter = _rhs.m_mipFilter;
	m_depthCmpFunc = _rhs.m_depthCmpFunc;
	m_borderHandling = _rhs.m_borderHandling;
#endif
	_rhs.m_id = 0;

	return *this;
}
