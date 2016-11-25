#version 440 core

// *** In and Outputs ***
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec3 in_bitangent;
layout(location = 4) in vec2 in_texCoord;

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
// TODO: add tangent and bitanget output
layout(location = 4) out vec2 out_texCoord;

// *** Buffers and Uniforms ***
layout(binding = 0, std140) uniform ubo_transform
{
	mat4 u_worldViewProjection;
	mat4 u_world;
	vec3 u_cameraPos;
	float u_swirl;
};


// *** Entry point ***
void main()
{
	// TODO: Rotate the position and all tangent space vectors around the Y-axis.
	// The given angle is dependent on the animation time and the y position.
	float angle = u_swirl * in_position.y;
	vec3 swirlPos = in_position;
	vec3 swirlNormal = in_normal;
	
	gl_Position = u_worldViewProjection * vec4(swirlPos, 1);
	out_normal = mat3(u_world) * swirlNormal;
	// TODO: add tangent and bitangent output
	out_position = mat4x3(u_world) * vec4(swirlPos, 1);
	out_texCoord = in_texCoord;
}
