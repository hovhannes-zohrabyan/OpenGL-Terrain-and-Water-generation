#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform float time;

out vec3 Normal;
out vec3 FragPos;
out mat4 modelToPass;
out mat4 viewToPass;
out mat4 projectionToPass;
out float timeToPass;

out VS_OUT
{
	vec2 TexCoord;
	int performWave;
} vs_out;



uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool performWave;


void main()
{
	modelToPass = model;
	viewToPass = view;
	projectionToPass = projection;
	timeToPass = time;


	if (performWave) {
		vs_out.performWave = 1;
	}

	gl_Position = vec4(aPos, 1.0f);
	vs_out.TexCoord = vec2(aTexCoord.x, aTexCoord.y);

	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(model))) * vec3(0, 1, 0);

}