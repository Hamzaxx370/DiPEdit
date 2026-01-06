#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;   // optional
layout (location = 2) in vec4 aWeight;   // optional
layout (location = 3) in vec2 aUV;       // optional
layout (location = 4) in vec4 aColor;       // optional

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 FragPos;
out vec3 Normal;
out vec2 UV;
out vec4 VertColor;

const int MAX_BONES = 100;
uniform mat4 finalBonesMatrices[MAX_BONES];

// optional usage flags
uniform bool hasNormals;
uniform bool hasWeights;
uniform bool hasUVs;
uniform bool hasColors;

void main()
{
    vec4 totalPosition = vec4(aPos, 1.0);
    vec3 skinnedNormal = aNormal;
    VertColor = vec4(1.0f);
    if (hasColors) {
        VertColor = aColor;
    }

    // Only do skinning if weights are provided
    if (hasWeights) {
        vec4 skinnedPos = vec4(0.0);
        vec3 normAccum = vec3(0.0);
    
        // Extract weights and bone indices
        float w0 = aWeight.x;
        float w1 = aWeight.y;
        int b0 = int(aWeight.z + 0.5); // convert float to int
        int b1 = int(aWeight.w + 0.5);
    
        if (w0 > 0.0 && b0 < MAX_BONES) {
            mat4 boneMatrix = finalBonesMatrices[b0];
            skinnedPos += (boneMatrix * vec4(aPos, 1.0)) * w0;
            normAccum += mat3(boneMatrix) * aNormal * w0;
        }
    
        if (w1 > 0.0 && b1 < MAX_BONES) {
            mat4 boneMatrix = finalBonesMatrices[b1];
            skinnedPos += (boneMatrix * vec4(aPos, 1.0)) * w1;
            normAccum += mat3(boneMatrix) * aNormal * w1;
        }
    
        totalPosition = skinnedPos;
        skinnedNormal = normalize(normAccum);
    }

    FragPos = vec3(model * totalPosition);

    // Use provided normal only if available
    Normal = hasNormals ? normalize(mat3(model) * skinnedNormal)
                        : vec3(0.0, 1.0, 0.0);

    // Default UV to 0 if missing
    UV = hasUVs ? aUV : vec2(0.0);

    gl_Position = proj * view * vec4(FragPos, 1.0);
}
