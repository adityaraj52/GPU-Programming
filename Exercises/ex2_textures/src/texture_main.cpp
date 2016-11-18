#include "../../shared/demowindow.hpp"
#include <gpuproframework.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

using namespace gpupro;
using namespace glm;


const char* VS_SCREEN_TRIANGLE = R"(
	#version 440 core

	layout(location = 0) out vec2 out_screenCoord;

	void main()
	{
		gl_Position.xy = vec2(float(gl_VertexID & 1) * 4.0 - 1.0,
							  float(gl_VertexID / 2) *-4.0 + 1.0);
		gl_Position.zw = vec2(1.0, 1.0);

		out_screenCoord = gl_Position.xy;
	}
)";

const char* PS_TEX_DEMO = R"(
	#version 440 core

	layout(location = 0) in vec2 in_screenCoord;
	layout(location = 0, index = 0) out vec3 out_fragColor;

	layout(location = 0) uniform float u_time;

	layout(binding = 0) uniform sampler2D tex_world;
	layout(binding = 1) uniform samplerCube tex_sky;

	vec3 fakeHDR(vec3 color)
	{
		//return color * pow(dot(color, vec3(0.4)), 1.5);
		return pow(color, vec3(1.8)) * 0.8 * length(color);
	}

	void main()
	{
		vec3 color = vec3(0.0);
		vec3 viewDir = normalize(vec3(in_screenCoord.xy, 2.0));

		// Rotate the viewDir time dependent
		float cosA = cos(u_time);
		float sinA = sin(u_time);
		viewDir.xz = mat2x2(cosA, -sinA, sinA, cosA) * viewDir.xz;

		// Perspective endless ground plane
		if(viewDir.y < 0.0)
			color = texture(tex_world, 0.2 * viewDir.xz / (-viewDir.y + 1e-2)).xyz;
		else
			color = fakeHDR(texture(tex_sky, viewDir).xyz);

		// Procedural sphere
		vec2 sphereCoord = (in_screenCoord + vec2(0.0, -0.0)) * 3.0;
		float r = dot(sphereCoord, sphereCoord);
		if(r < 1.0)
		{
			sphereCoord *= 0.95;	// Perspective correction
			float sphereZ = sqrt(1.0 - dot(sphereCoord, sphereCoord));
			vec3 sphereNormal = vec3(sphereCoord, -sphereZ);
			sphereNormal.xz = mat2x2(cosA, -sinA, sinA, cosA) * sphereNormal.xz;
			vec3 reflectedDir = reflect(viewDir, sphereNormal);
			if(reflectedDir.y < 0.0)
				color = texture(tex_world, viewDir.xz * (1 - sphereZ) + 0.2 * reflectedDir.xz / (-reflectedDir.y + 1e-2)).xyz;
			else
				color = fakeHDR(texture(tex_sky, reflectedDir).xyz);
			color *= abs(dot(viewDir, sphereNormal)) * 0.5 + 0.5;
		}

		out_fragColor = color;
	}
)";


static int s_currentSampler = 0;
static bool s_anisotropicSampling = false;
static bool s_rotating = true;
static void keyFunc(GLFWwindow* _window, int _key, int, int _action, int)
{
	if(_action == GLFW_PRESS)
	{
		switch(_key)
		{
			case GLFW_KEY_RIGHT: s_currentSampler = (s_currentSampler + 1) % 4; break;
			case GLFW_KEY_LEFT: s_currentSampler = (s_currentSampler + 3) % 4; break;
			case GLFW_KEY_A: s_anisotropicSampling = !s_anisotropicSampling; break;
			case GLFW_KEY_R: s_rotating = !s_rotating; break;
			case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(_window, GLFW_TRUE); break;
		}
	}
}

int main()
{
	std::cerr << "GPU Programming Exercise 2                          TU Clausthal\n"
		<< "                                         Johannes Jendersie 2016\n"
		<< "----------------------------------------------------------------\n"
		<< "In this exercise various sampler states are demonstrated.\n"
		<< "----------------------------------------------------------------\n"
		<< "Key bindings:\n"
		<< "  Escape:     quit program\n"
		<< "  A:          toggle anisotropic filtering\n"
		<< "  R:          toggle rotation\n"
		<< "  Left/Right: change the used sampler\n\n";

	try {
		DemoWindow window(1024, 1024, "Ex2: Textures");
		OGLContext context(OGLContext::DebugSeverity::MEDIUM);
		window.setKeyCallback(keyFunc);
		glViewport(0, 0, 1024, 1024);

		// Create a pipeline which draws a screen filling triangle
		// and shows some effects for textures.
		Pipeline texDemoPipe;
		Shader simpleVert(Shader::Type::VERTEX);
		Shader simpleFrag(Shader::Type::FRAGMENT);
		simpleVert.loadFromSource(VS_SCREEN_TRIANGLE);
		simpleFrag.loadFromSource(PS_TEX_DEMO);
		Program simpleShader;
		simpleShader.attach(simpleVert);
		simpleShader.attach(simpleFrag);
		simpleShader.link();
		texDemoPipe.shader = &simpleShader;

		// TODO: Create and load the 2D RGB8 texture "world.png"
		Texture tex1(Texture::Layout::TEX_2D,InternalFormat::RGB8);
		tex1.load("world.png");

		// TODO: Create and load the cube map RGB8 texture "sky".
		// To load a cube map create the texture object first and call
		// load() six times. The second parameter _layer lets you
		// specify into which cubemap face the file is loaded. Load
		// xpos to 0, xneg to 1, ypos to 2, ....
		Texture tex2(Texture::Layout::TEX_2D, InternalFormat::RGB8);
		tex2.load("sky",0);
		tex2.load("sky",1);
		tex2.load("sky",2);
		tex2.load("sky",3);
		tex2.load("sky",4);
		tex2.load("sky",5);


		// Create some samplers
		const char* SAMPLER_NAMES[] = {
			"NEAREST min/mag, NO mip-map",
			"NEAREST min/mag/mip",
			"NEAREST min, LINEAR mag/mip (GL default)",
			"LINEAR min/mag/mip"
		};
		std::vector<SamplerState> samplers;
		// TODO: Create the four sampler states as described by SAMPLER_NAMES.
		SamplerState s1(SamplerState::Filter::NEAREST, SamplerState::Filter::NEAREST, SamplerState::Filter::NONE);
		SamplerState s2(SamplerState::Filter::NEAREST, SamplerState::Filter::NEAREST, SamplerState::Filter::NEAREST);
		SamplerState s3(SamplerState::Filter::NEAREST, SamplerState::Filter::LINEAR, SamplerState::Filter::LINEAR);
		SamplerState s4(SamplerState::Filter::LINEAR, SamplerState::Filter::LINEAR, SamplerState::Filter::LINEAR);


		// Use samplers.push_back(SamplerState(...)) to add these states into the samplers array.
		samplers.push_back(s1);
		samplers.push_back(s2);
		samplers.push_back(s3);
		samplers.push_back(s4);

		
		float maxAnisotropy = 1.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
		// TODO: Create the same four samplers but use the maxAnisotropy this 
		/*
		SamplerState s5(SamplerState::Filter::NEAREST, SamplerState::Filter::NEAREST, SamplerState::Filter::NONE, maxAnisotropy);
		SamplerState s6(SamplerState::Filter::NEAREST, SamplerState::Filter::NEAREST, SamplerState::Filter::NEAREST, maxAnisotropy);
		SamplerState s7(SamplerState::Filter::NEAREST, SamplerState::Filter::LINEAR, SamplerState::Filter::LINEAR, maxAnisotropy);
		SamplerState s8(SamplerState::Filter::LINEAR, SamplerState::Filter::LINEAR, SamplerState::Filter::LINEAR, maxAnisotropy);
		*/
		// Main loop
		float time = 0.0f;
		while(window.isOpen())
		{
			// Enable pipeline
			context.setState(texDemoPipe);

			// TODO: Bind the two textures (world to 0 and sky to 1)
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindTexture(GL_TEXTURE_2D, 1);

			// Draw some vertices. Even though there is no buffer the vertex shader
			// will be called 3 times. Then, positions are generated inside the shader.
			glUniform1f(0, time);
			glDrawArrays(GL_TRIANGLES, 0, 3);

			// Input handling
			window.handleEventsAndPresent();
			// TODO: uncomment the two lines if the samplers-setup is completed.
			texDemoPipe.samplerState[0] = &samplers[s_currentSampler + (s_anisotropicSampling ? 4 : 0)];
			texDemoPipe.samplerState[1] = &samplers[s_currentSampler + (s_anisotropicSampling ? 4 : 0)];
			if(s_anisotropicSampling)
				printf("\rCurrent sampler: %s (anisotropic)                      ", SAMPLER_NAMES[s_currentSampler]);
			else
				printf("\rCurrent sampler: %s                                    ", SAMPLER_NAMES[s_currentSampler]);

			if(s_rotating) time += 0.001f;
		}
	} catch(std::exception _ex) {
		std::cerr << "ERR: " << _ex.what();
		return 1;
	}

	return 0;
}