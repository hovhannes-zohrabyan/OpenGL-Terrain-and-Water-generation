#version 330 core
out vec4 FragColor;

in GS_OUT
{
    vec2 TexCoord;
} fs_in;

// texture samplers
uniform sampler2D textureMain;
uniform sampler2D DudvMap;
uniform sampler2D Reflection;

in vec3 NormalG;
in vec3 FragPosG;

uniform vec3 lightPos = vec3(-10.0f, 1.0f, 2.0f);
uniform vec3 viewPos;
uniform vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

uniform float offset;

void main()
{
	vec2 distortion1 = (texture(DudvMap, fs_in.TexCoord).rg * 2.0 - 1.0) * 0.9;
    vec2 coord = fs_in.TexCoord * vec2(5,5);
	coord += vec2(offset, 0);
	coord += distortion1;

    // ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse 
    vec3 norm = normalize(NormalG);
    vec3 lightDir = normalize(lightPos - FragPosG);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient) * vec3(0.0f, 0.0f, 1.0f);
    FragColor = mix(texture(textureMain, coord), vec4(result, 0.3), 0.2);

    //FragColor = texture(Reflection, fs_in.TexCoord);
}



