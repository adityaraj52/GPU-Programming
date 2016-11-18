#include "../../shared/demowindow.hpp"
#include <gpuproframework.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

using namespace gpupro;
using namespace glm;

// From file fractalgen.cpp
void genFractal(int _levels, float _scale, vec3 _translation, std::vector<vec3>& _positions, std::vector<vec3>& _normals, std::vector<unsigned>& _indices);


struct TransformUniforms
{
	mat4 worldViewProjection;
	mat4 world;
};

const char* VS_SIMPLE = R"(
	#version 440 core

	layout(location = 0) in vec3 in_position;
	layout(location = 1) in vec3 in_normal;
	layout(location = 0) out vec3 out_position;
	layout(location = 1) out vec3 out_normal;

	layout(binding = 0) uniform ubo_transform
	{
		mat4 u_worldViewProjection;
		mat4 u_world;
	};

	void main()
	{
		gl_Position = u_worldViewProjection * vec4(in_position, 1);
		out_normal = mat3(u_world) * in_normal;
		out_position = mat4x3(u_world) * vec4(in_position, 1);
	}
)";

const char* PS_SIMPLE = R"(
	#version 440 core

	layout(location = 0) in vec3 in_position;
	layout(location = 1) in vec3 in_normal;

	layout(location = 0, index = 0) out vec3 out_fragColor;

	uniform vec3 LIGTH_DIR = vec3(0.267261242, 0.801783726, -0.534522484);

	void main()
	{
		float light = dot(LIGTH_DIR, in_normal);
		light = light * 0.5 + 0.5;

		float centerShade = length(in_position) * 0.65 + 0.2;
		light *= centerShade;

		float shadow = clamp(dot(LIGTH_DIR, in_position) / 2.0 + 0.2, 0.2, 1.0);
		light *= shadow;

		out_fragColor = vec3(light);
	}
)";

static void keyFunc(GLFWwindow* _window, int _key, int, int _action, int)
{
	if(_action == GLFW_PRESS && _key == GLFW_KEY_ESCAPE)
	{
		glfwSetWindowShouldClose(_window, GLFW_TRUE);
	}
}


int main()
{
	std::cerr << "GPU Programming Exercise 1                          TU Clausthal\n"
		<< "                                         Johannes Jendersie 2016\n"
		<< "----------------------------------------------------------------\n"
		<< "This is a demo of vertex buffers, vertex formats index buffers\n"
		<< "and uniform buffers. These types of buffers are used to draw an\n"
		<< "high poly mesh (generated fractal).\n"
		<< "----------------------------------------------------------------\n"
		<< "Key bindings:\n"
		<< "  Escape:     quit program\n\n";

		DemoWindow window(1024, 1024, "Ex1: buffer - fractal rendering");
		OGLContext context(OGLContext::DebugSeverity::MEDIUM);
		window.setKeyCallback(keyFunc);

		glClearColor(0.1f, 0.4f, 0.05f, 0.0f);
		glViewport(0, 0, 1024, 1024);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		// Load a simple draw pipeline with procedural lighting
		Pipeline drawModelPipe;
		Shader simpleVert(Shader::Type::VERTEX);
		Shader simpleFrag(Shader::Type::FRAGMENT);
		simpleVert.loadFromSource(VS_SIMPLE);
		simpleFrag.loadFromSource(PS_SIMPLE);
		Program simpleShader(simpleVert, simpleFrag);
		drawModelPipe.shader = &simpleShader;

		// Generate and upload data.
		// For generation use a fractal level of 0 to 4. Level 5 requires more than 7GB.
		std::vector<vec3> positions;
		std::vector<vec3> normals;
		std::vector<GLuint> indices;
		genFractal(3, 1.0f, vec3(0.0f), positions, normals, indices);
		std::cerr << "INF: Generated the fractal with " << positions.size() << " vertices and " << indices.size() << " indices.\n";
		
		// TODO: Create two vertex buffers and one index buffer using the data
		// from the three generated arrays. They should have static usage 'Buffer::Usage()'.

		// TODO: Define a vertex format with one vec3 from vertex buffer 0 (positions)
		// and one vec3 from vertex buffer 1 (normals).

		Buffer bo1(Buffer::VERTEX, sizeof(vec3), positions.size(), Buffer::Usage(), positions.data());
		Buffer bo2(Buffer::VERTEX, sizeof(vec3), normals.size(), Buffer::Usage(), normals.data());

		Buffer bo3(Buffer::VERTEX, sizeof(GLuint), indices.size(), Buffer::Usage(), indices.data());

		VertexFormat meshFormat({
			VertexAttribute{0, 0, 3, VertexAttribute::Type::FLOAT, GL_FALSE, 0, 0},
			VertexAttribute{1, 1, 3, VertexAttribute::Type::FLOAT, GL_FALSE, 0, 0}
		});



		drawModelPipe.vertexFormat = &meshFormat;

		// Create a uniform buffer for transformations
		// TODO: Create a uniform buffer with sizeof(TransformUniforms) and with
		// sub-data updates enabled. It has one element.
		Buffer ubo(Buffer::UNIFORM, sizeof(TransformUniforms), 1, Buffer::Usage::SUB_DATA_UPDATE, NULL);


		// Main loop
		float rotation = 0.0f;
		while(window.isOpen())
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Fill uniform buffer
			TransformUniforms uniforms;
			uniforms.world = glm::rotate(mat4(1.0f), rotation, vec3(0.0f, 1.0f, 0.0f));
			uniforms.worldViewProjection = glm::perspective(40.0f * 3.1415926f / 180.0f, 1.0f, 0.1f, 100.0f)
				* glm::lookAt(vec3(0.0f, 2.0f, -6.0f), vec3(0.0f), vec3(0.0f, 1.0f, 0.0f))
				* uniforms.world;
			// TODO: update the uniform buffer.
			ubo.bindAsUniformBuffer(0, 0);
			ubo.subDataUpdate(0, sizeof(TransformUniforms), &uniforms);

			// Enable pipeline
			context.setState(drawModelPipe);

			// TODO: Bind buffer data
			bo1.bindAsVertexBuffer(0, 0);
			bo2.bindAsVertexBuffer(1, 0);
			bo3.bindAsIndexBuffer();
			//glBindVertexArray(meshFormat.glID());

			// TODO: Make the indexed draw call glDrawElements.
			// The 'count' in the draw call is the number of indices (not triangles).
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

			window.handleEventsAndPresent();

			rotation += 0.002f;
		}
	return 0;
}