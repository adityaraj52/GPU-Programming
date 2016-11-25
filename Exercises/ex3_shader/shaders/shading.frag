#version 440 core

// *** In and Outputs ***
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
// TODO: Declare the inputs for tangent and bitangent.
layout(location = 4) in vec2 in_texCoord;

layout(location = 0, index = 0) out vec4 out_fragColor;

// *** Textures ***
layout(binding = 0) uniform sampler2D tex_albedo;
layout(binding = 1) uniform sampler2D tex_normal;
layout(binding = 2) uniform sampler2D tex_specular;

// *** Buffers and Uniforms ***
layout(binding = 0, std140) uniform ubo_transform
{
	mat4 u_worldViewProjection;
	mat4 u_world;
	vec3 u_cameraPos;
	float u_swirl;
};

layout(binding = 1) uniform ubo_shading
{
	vec4 u_lightPosition[8];
	vec4 u_lightColor[8];
	bool u_normalMapping;
};

uniform vec3 LIGHT_DIR = vec3(0.267261242, 0.801783726, -0.534522484);

// *** Functions ***
// Blinn-Phong like lighting
void lighting(in vec3 normal, in vec3 toLight, in vec3 viewDir, in vec3 intensity, in float specPower, inout vec3 diffuse, inout vec3 specular)
{
	float cosTheta = max(dot(toLight, normal), 0.0);
	vec3 halfVec = normalize(toLight + viewDir);
	diffuse += cosTheta * intensity;
	specular += cosTheta * pow(max(0.0, dot(halfVec, normal)), specPower) * intensity;
}

// *** Entry point ***
void main()
{
	// Normal mapping: load the local normal from the texture and transform
	// it to global space.
	// If u_normalMapping is false disable normal mapping.
	vec3 normal;
	if(u_normalMapping)
	{
		// The normal map is loaded as signed texture. It is already in [-1,1]
		// as required. The x and y components of the normal map align with
		// in_tangent and in_bitangent and z with the original in_normal.
		// TODO: Implement normal mapping here.
		normal = in_normal;
	} else
		normal = in_normal;
	normal = normalize(normal);
	
	// Compute and read properties which are required for lighting.
	vec3 viewDir = normalize(u_cameraPos - in_position);
	vec2 spec_roughness = texture(tex_specular, in_texCoord.xy).xy;
	float power = 1.0 / (spec_roughness.y * spec_roughness.y);
	vec3 diffuseColor = texture(tex_albedo, in_texCoord.xy).xyz;
	
	// Lighting.
	vec3 diffLight = vec3(0.0);
	vec3 specLight = vec3(0.0);
	lighting(normal, LIGHT_DIR, viewDir, vec3(0.6), power, diffLight, specLight);
	for(int i = 0; i < 8; ++i)
	{
		// TODO: Compute the direction and the distance from the current position
		// to the point light (u_lightPosition[i]).
		vec3 toLight = vec3(0.0);

		// TODO: Compute the light intensity based on its color and the distance to the light source.
		// You can use a linear or quadratic (physical) falloff. Without tone mapping a linear
		// falloff is easier to control and often looks better.
		vec3 intensity = u_lightColor[i].xyz /*do things here*/;
		
		// Call the lighting equation for the point light
		lighting(normal, toLight, viewDir, intensity, power, diffLight, specLight);
	}
	
	// Combine diffuse and specular term dependent on the view-angle (more reflection
	// on flat angles). Also, diffuse lighting is reduced in flat view-angles.
	float specLevel = spec_roughness.x * (0.6 + 0.4 * pow(1 - max(0.0, dot(normal, viewDir)), 5.0));
	out_fragColor.rgb = diffuseColor * diffLight * (1-specLevel) + specLight * specLevel;
	// Write the specular intensity into alpha, since it is used for reflection masking or ignored anyway.
	out_fragColor.a = specLevel;
}