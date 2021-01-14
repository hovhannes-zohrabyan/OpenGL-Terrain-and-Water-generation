#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


uniform sampler2D textureMainGround;
uniform sampler2D heightMap;

void main()
{
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);

	float height_data = texture(heightMap, aTexCoord).r;

	vec3 newCoord = vec3(aPos.x, (aPos.y + height_data) - 0.5, aPos.z);

	gl_Position = projection * view * model * vec4(newCoord, 1.0f);


	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(model))) * vec3(0, 1, 0);

}