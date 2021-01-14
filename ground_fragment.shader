#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D textureMainGround;
uniform sampler2D heightMap;


in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos = vec3(-10.0f, 1.0f, 2.0f);
uniform vec3 viewPos;
uniform vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

void main()
{
	vec2 coord = TexCoord*vec2(1,1);

    // ambient
    float ambientStrength = 2;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * vec3(0.0f, 0.0f, 1.0f);
    FragColor = texture(textureMainGround, coord);
}



