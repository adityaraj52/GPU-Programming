#include "../../shared/demowindow.hpp"
#include <gpuproframework.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

using namespace gpupro;
using namespace glm;


struct TransformUniforms
{
	mat4 worldViewProjection;
	mat4 world;
	vec3 cameraPosition;
	float swirl;
};

struct ShadingUniforms
{
	vec4 lightPosition[8];
	vec4 lightColor[8];
	unsigned normalMapping;
};

const vec3 lightColors[8] = {
	vec3(1.0, 1.0, 1.0),
	vec3(0.1, 0.1, 0.5),
	vec3(0.5, 0.5, 0.5),
	vec3(0.1, 0.5, 0.1),
	vec3(2.0, 2.0, 2.0),
	vec3(0.8, 0.1, 0.1),
	vec3(1.0, 1.0, 1.0),
	vec3(1.0, 1.0, 0.0),
};


static bool s_normalMapping = true;
static bool s_swirl = false;
static void keyFunc(GLFWwindow* _window, int _key, int, int _action, int)
{
	if(_action == GLFW_PRESS)
	{
		switch(_key)
		{
			case GLFW_KEY_N: s_normalMapping = !s_normalMapping; break;
			case GLFW_KEY_S: s_swirl = !s_swirl; break;
			case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(_window, GLFW_TRUE);
		}
	}
}

static float s_camTheta = 0.5f;
static float s_camPhi = 0.0f;
static void mouseFunc(GLFWwindow* _window, double _x, double _y)
{
	static double oldX, oldY;
	if(glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		float deltaX = float(_x - oldX);
		float deltaY = float(_y - oldY);

		s_camPhi -= deltaX * 0.007f;
		s_camTheta = clamp(s_camTheta + deltaY * 0.007f, -1.5f, 1.5f);
	}
	oldX = _x;
	oldY = _y;
}

static float s_camZoom = 20.0f;
static void scrollFunc(GLFWwindow *, double , double _sy)
{
	s_camZoom = clamp(s_camZoom - 2*(float)_sy, 10.0f, 40.0f);
}


int main()
{
	std::cerr << "GPU Programming Exercise 3                          TU Clausthal\n"
		<< "                                         Johannes Jendersie 2016\n"
		<< "----------------------------------------------------------------\n"
		<< "In this demo a few shaders and states are combined to obtain\n"
		<< "various effects: lighting, normal mapping and reflections.\n"
		<< "----------------------------------------------------------------\n"
		<< "Key bindings:\n"
		<< "  Escape:     quit program\n"
		<< "  N:          toggle normal map\n"
		<< "  S:          toggle swirl transformation of the teapot\n"
		<< "  Mouse:      change camera rotation (press left button)\n"
		<< "              zoom (wheel)\n\n";

	try {
		DemoWindow window(1024, 1024, "Ex3: Shaders and States");
		OGLContext context(OGLContext::DebugSeverity::LOW);
		window.setKeyCallback(keyFunc);
		window.setMouseCallback(mouseFunc);
		window.setScrollCallback(scrollFunc);
		glViewport(0, 0, 1024, 1024);

		// TODO: Create a standard rendering pipeline with back-face culling and depth testing.
		Pipeline objectShadingWithSwirlPipe;
		objectShadingWithSwirlPipe.rasterizer.cullMode = gpupro::RasterizerState::CullMode::BACK;
		objectShadingWithSwirlPipe.depthStencil.depthTest = true;

		// The following pipelines must be changed for Task4. You can ignore them
		// for the first 3 tasks...

		// Setup a pipeline to show the plane with z-test. It has a special blending,
		// such that the intensity of reflected objects is controlled by SRC_ALPHA.
		// Use additive blending with SRC = ONE and DST = SRC_ALPHA.
		Pipeline planeShadingPipe;
		planeShadingPipe.depthStencil.depthTest = true;
		// TODO: enable blending as described above.
		planeShadingPipe.blendState.enableBlending = gpupro::BlendState::BlendMode::BLEND;

		// TODO: Setup a pipeline which writes 1 into the stencil buffer where an object
		// with z-testing is drawn. This state must not write into color or depth.
		Pipeline setStencilPipe;
		

		// Finally, we need the another depth test + culling object pipeline, which only
		// draws where the stencil buffer is 1. Also, flip the face winding because of the mirroring!
		Pipeline objectShadingWithSwirlMaskedPipe;
		objectShadingWithSwirlMaskedPipe = objectShadingWithSwirlPipe;
		// TODO: make the settings to the stencil buffer comparison and the face winding.

		// glClear() uses the current state, which is a bad thing if some writes
		// are disabled or masked. So use a simple default pipeline for that.
		Pipeline clearState;

		// Load shaders
		Shader simpleVert(Shader::Type::VERTEX, "shaders/simple.vert");
		Shader swirlVert(Shader::Type::VERTEX, "shaders/swirl.vert");
		Shader shadingFrag(Shader::Type::FRAGMENT, "shaders/shading.frag");
		Program standardShader(simpleVert, shadingFrag);
		Program swirlShader(swirlVert, shadingFrag);
		setStencilPipe.shader = &standardShader;
		planeShadingPipe.shader = &standardShader;
		objectShadingWithSwirlPipe.shader = &swirlShader;
		objectShadingWithSwirlMaskedPipe.shader = &swirlShader;

		// Load the textures
		Texture metalDiff(InternalFormat::RGB8, "model/brushed_metal_diff.png");
		Texture metalNorm(InternalFormat::RGB8S, "model/brushed_metal_norm.png");
		Texture metalSpec(InternalFormat::RGB8, "model/brushed_metal_spec.png");
		Texture cobbleDiff(InternalFormat::RGB8, "model/cobblestone_diff.png");
		Texture cobbleNorm(InternalFormat::RGB8S, "model/cobblestone_norm.png");
		Texture cobbleSpec(InternalFormat::RGB8, "model/cobblestone_spec.png");
		// Create and set a sampler
		SamplerState niceSampler(SamplerState::Filter::LINEAR, SamplerState::Filter::LINEAR, SamplerState::Filter::LINEAR, 8.0f);
		for(int i = 0; i < 3; ++i)
		{
			planeShadingPipe.samplerState[i] = &niceSampler;
			objectShadingWithSwirlPipe.samplerState[i] = &niceSampler;
			objectShadingWithSwirlMaskedPipe.samplerState[i] = &niceSampler;
		}

		// Create the vertex format
		std::vector<VertexAttribute> attributes({
			{0, 0, 3, VertexAttribute::Type::FLOAT, GL_FALSE, 0, 0},	// Position
			{1, 1, 3, VertexAttribute::Type::FLOAT, GL_FALSE, 0, 0},	// Normal
			{2, 1, 3, VertexAttribute::Type::FLOAT, GL_FALSE, 12, 0},	// Tangent
			{3, 1, 3, VertexAttribute::Type::FLOAT, GL_FALSE, 24, 0},	// Bitangent
			{4, 2, 2, VertexAttribute::Type::FLOAT, GL_FALSE, 0, 0}		// TexCoord
		});
		VertexFormat vertexFormat(attributes);
		setStencilPipe.vertexFormat = &vertexFormat;
		planeShadingPipe.vertexFormat = &vertexFormat;
		objectShadingWithSwirlPipe.vertexFormat = &vertexFormat;
		objectShadingWithSwirlMaskedPipe.vertexFormat = &vertexFormat;

		// Load objects
		OBJLoader objloader;
		objloader.load("model/teapot.obj", true);
		Model teapot(objloader);
		objloader.load("model/plane.obj", true);
		Model plane(objloader);

		// Create a uniform buffers
		Buffer transformUBO(Buffer::Type::UNIFORM, sizeof(TransformUniforms), 1, Buffer::Usage::SUB_DATA_UPDATE);
		Buffer shadingUBO(Buffer::Type::UNIFORM, sizeof(ShadingUniforms), 1, Buffer::Usage::SUB_DATA_UPDATE);
	
		// Main loop
		float animation = 0.0f;
		while(window.isOpen())
		{
			context.setState(clearState);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			// Fill uniform buffers
			TransformUniforms uniforms;
			uniforms.cameraPosition = vec3(sin(s_camPhi) * cos(s_camTheta), sin(s_camTheta), cos(s_camPhi) * cos(s_camTheta)) * s_camZoom;
			mat4 viewProjection = glm::perspective(40.0f * 3.1415926f / 180.0f, 1.0f, 0.1f, 100.0f)
				* glm::lookAt(uniforms.cameraPosition, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
			uniforms.world = glm::mat4(1.0f);
			uniforms.worldViewProjection = viewProjection;
			uniforms.swirl = s_swirl ? sin(animation * 2.0f) * 0.5f : 0.0f;
			transformUBO.subDataUpdate(0, sizeof(TransformUniforms), &uniforms);

			// Set animated light sources
			ShadingUniforms lightUniforms;
			for(int i = 0; i < 8; ++i)
			{
				float angle = (i / 8.0f) * 2 * pi<float>() + animation;
				lightUniforms.lightPosition[i] = vec4(sin(angle) * 5.0f, 1.0f, cos(angle) * 5.0f, 0.0f);
				float intensity = sin(animation * 34.31f + i) * 0.2f + 2.5f;
				lightUniforms.lightColor[i] = vec4(intensity * lightColors[i], 0.0f);
			}
			lightUniforms.normalMapping = s_normalMapping;
			shadingUBO.subDataUpdate(0, sizeof(ShadingUniforms), &lightUniforms);

			// Draw the scene
			transformUBO.bindAsUniformBuffer(0);
			shadingUBO.bindAsUniformBuffer(1);
			context.setState(objectShadingWithSwirlPipe);
			teapot.bind(0, 1, 2);
			metalDiff.bindAsTexture(0);
			metalNorm.bindAsTexture(1);
			metalSpec.bindAsTexture(2);
			teapot.draw();

			// TODO: Draw the mirror plane into the stencil buffer using setStencilPipe.

			// TODO: Mirror the camera at xz-plane. Therefore, you need to multiply
			// the 'viewProjection' with a reflection matrix and update the transformUBO.

			// TODO: Draw mirrored object using the objectShadingWithSwirlMaskedPipe.

			// Draw the plane itself
			context.setState(planeShadingPipe);
			plane.bind(0, 1, 2);
			cobbleDiff.bindAsTexture(0);
			cobbleNorm.bindAsTexture(1);
			cobbleSpec.bindAsTexture(2);
			plane.draw();

			// Input handling
			window.handleEventsAndPresent();
			animation += 0.002f;
		}
	} catch(std::exception _ex) {
		std::cerr << "ERR: " << _ex.what();
		return 1;
	}

	return 0;
}