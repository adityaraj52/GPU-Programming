#pragma once

#include "program.hpp"
#include "vertexformat.hpp"

#include "gl.hpp"

namespace gpupro {

	// Sampler states are real resources which must be allocated...
	// Create some sampler objects globally and share them everywhere.
	// You will probably never need more than 3 different samplers.
	class SamplerState
	{
	public:
		enum struct BorderHandling {
			REPEAT = GL_REPEAT,
			CLAMP = GL_CLAMP_TO_EDGE,
			BORDER = GL_CLAMP_TO_BORDER,
			MIRROR = GL_MIRRORED_REPEAT
		};

		enum struct Filter {
			NONE,				///< No filtering is only valid for mip-filter (i.e. mip-mapping can be disabled)
			NEAREST = GL_NEAREST,
			LINEAR = GL_LINEAR
		};

		// Use depth comparison for textures with a depth-format (shadow mapping).
		// For all other textures choose DISABLE.
		enum struct DepthCompareFunc {
			LESS = GL_LESS,
			LESS_EQUAL = GL_LEQUAL,
			GREATER = GL_GREATER,
			GREATER_EQUAL = GL_GEQUAL,
			EQUAL = GL_EQUAL,
			NOT_EQUAL = GL_NOTEQUAL,
			NEVER = GL_NEVER,
			ALWAYS = GL_ALWAYS,
			DISABLE
		};

		// The constructor creates the entire sampler object with all parameters.
		// The arguments are sorted for importance and the defaults are that
		// of OpenGL.
		// _maxAnisotropy: A number between 1.0f and GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT. 1.0f disables
		//		anisotropic filtering.
		SamplerState(Filter _minFilter = Filter::NEAREST,
			Filter _magFilter = Filter::LINEAR,
			Filter _mipFilter = Filter::LINEAR,
			float _maxAnisotropy = 1.0f,
			DepthCompareFunc _depthCmpFunc = DepthCompareFunc::DISABLE,
			BorderHandling _borderHandling = BorderHandling::REPEAT,
			float _borderColor[4] = nullptr
		);
		~SamplerState();
		// Move but not copy-able
		SamplerState(SamplerState&& _rhs);
		SamplerState(const SamplerState&) = delete;
		SamplerState& operator = (SamplerState&& _rhs);
		SamplerState& operator = (const SamplerState& _rhs) = delete;

		GLuint glID() { return m_id; }
	private:
		GLuint m_id;
		// The following states are not required for execution.
		// They only expose the state for debugging reasons.
#ifdef DEBUG
		Filter m_minFilter;
		Filter m_magFilter;
		Filter m_mipFilter;
		DepthCompareFunc m_depthCmpFunc;
		// It is possible to use different handlers in different dimensions.
		// Since there are practically no usecases this framework simplifies
		// this down to one setting.
		BorderHandling m_borderHandling;
#endif
	};

	// A pipeline is a collection of the entire OpenGL state (it will grow
	// to that in the next few weeks).
	// Further, it bundles a shader, a vertex format together
	// with the state.
	//
	// The reasons for the monolithic state object are:
	//	* Robustness in large projects against human made mistakes.
	//	  When rendering in multiple passes each pass has its own fixed
	//    state. Changing other passes will not influence its behavior.
	//	* Simplicity
	//	  You don't need to find all those different glEnable, glMask...
	//	  commands. Also, you have an overview of the standard values which
	//	  are set in OpenGL initially.
	//  * Performance (theoretical).
	//	  The single state object could be validated outside the render loop
	//	  and maybe even off-line (install time) making its binding at runtime
	//	  cheaper. Unfortunately, this is not yet supported by OpenGL.
	//	  Therefore, this is simulated only in the framework. The
	//	  OGLContext::setState() will do as few as possible real state changes
	//	  on GPU side. However, each state is tested if it changed since the
	//	  previous state. All together this can be faster or slower than a raw
	//	  implementation with many states.
	//	* API upward compatibility DX12 and Vulkan follow the same concept.
	struct Pipeline
	{
		// 64 textures is very high. It might be that your GPU does not support
		// that many. Have a look at GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS.
		SamplerState* samplerState[64] = {nullptr};
		Program* shader = nullptr;
		VertexFormat* vertexFormat = nullptr;
	};

} // namespace gpupro