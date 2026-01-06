#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aWeight;
layout (location = 3) in ivec4 aWeightIndex;
layout (location = 4) in vec2 aUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 FragPos;
out vec3 Normal;
out vec2 UV;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
	vec4 totalPosition = vec4(0.0f);
    vec3 skinnedNormal = vec3(0.0f);
    for(int i = 0 ; i < 2 ; i++)
    {
        if(aWeight[i] == 0.0) 
            continue;
        if(aWeightIndex[i] >= MAX_BONES)
            break;
        
        mat4 boneMatrix = finalBonesMatrices[aWeightIndex[i]];

        totalPosition += (boneMatrix * vec4(aPos, 1.0f)) * aWeight[i];

        skinnedNormal += mat3(boneMatrix) * aNormal * aWeight[i];
    }

    FragPos = vec3(model * totalPosition);
    Normal = normalize(mat3(model) * skinnedNormal);
    UV = vec2(aUV.x, aUV.y);
     
    gl_Position = proj * view * vec4(FragPos, 1.0f);
}