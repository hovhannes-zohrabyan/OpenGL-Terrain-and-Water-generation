#version 430 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;


in VS_OUT
{
    vec2 TexCoord;
    int performWave;

} gs_in_p[];

out GS_OUT
{
    vec2 TexCoord;
} gs_out;


in vec3 Normal[];
in vec3 FragPos[];
in mat4 modelToPass[];
in mat4 viewToPass[];
in mat4 projectionToPass[];
in float timeToPass[];



float amp = 0.4f;
float waveLenght = 10.0f;

out vec2 TexCoordG;
out vec3 NormalG;
out vec3 FragPosG;

uniform vec3 rippleCenter[];

uniform vec4 plane = vec4(0, 1, 0, 0);



void createWave(vec4 position, int vetrexIndex) {
    float centerX = -rippleCenter[vetrexIndex].x*0.8;
    float centerZ = rippleCenter[vetrexIndex].z*0.8;

    float distanceFromCenter = sqrt(pow((position.x - centerX), 2.0) + pow((position.z - centerZ), 2.0));
    float y_pos = (sin(distanceFromCenter* waveLenght - timeToPass[0]*10.0) / (distanceFromCenter * waveLenght - timeToPass[0] * 10.0))*amp;

    position = position + vec4(0.0, y_pos, 0.0, 0.0);
    position = projectionToPass[vetrexIndex] * viewToPass[vetrexIndex] * modelToPass[vetrexIndex] * position;
    gl_Position = position;
    gs_out.TexCoord = gs_in_p[vetrexIndex].TexCoord;

    //vec4 worldPosition = gl_in[vetrexIndex].gl_Position;
    //gl_ClipDistance[0] = dot(worldPosition, plane);

    EmitVertex();
}

void main() {

    if (gs_in_p[0].performWave == 1) {
        createWave(gl_in[0].gl_Position, 0);

        createWave(gl_in[1].gl_Position, 1);

        createWave(gl_in[2].gl_Position, 2);
    
    }

    NormalG = mat3(transpose(inverse(modelToPass[1]))) * vec3(0, 1, 0);
    FragPosG = FragPos[0];

    EndPrimitive();
}
