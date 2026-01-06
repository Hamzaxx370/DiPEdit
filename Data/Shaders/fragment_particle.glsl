#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;
in vec4 VertColor;

struct LightPoint {
    vec3 pos;
    vec3 dir;
    vec3 color;
    float intensity;
    float radius;
};

uniform sampler2D tex0;
uniform LightPoint Lights[100];
uniform int LightCount;
uniform mat4 view;

uniform vec2 uvBase;
uniform vec2 uvScale;
uniform vec4 colorBase;

// feature flags
uniform bool hasNormals;
uniform bool hasUVs;

void main()
{
    vec4 TexColor = hasUVs ? texture(tex0, (UV * uvScale)+ uvBase) : vec4(0.5,0.5,0.5,1.0);

    vec3 objectColor = VertColor.rgb * TexColor.rgb * colorBase.rgb;
    float alpha = 1.0f;

    if ( hasUVs ) {
        alpha = TexColor.a;
    }

    alpha *= colorBase.a;
    alpha *= VertColor.a;

    // Default simple unlit color when no normals
    if (!hasNormals || LightCount == 0) {
        FragColor = vec4(objectColor, alpha);
        return;
    }

    vec3 viewPos = (inverse(view))[3].xyz;
    vec3 norm = normalize(Normal);
    float specStrength = 0.5;        
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * objectColor;
    vec3 lighting = ambient;
    for (int i = 0; i < LightCount; i++) {
        vec3 lightDir = normalize(Lights[i].pos - FragPos);
        float diff = max(dot(norm,lightDir),0.0);
        vec3 diffuse = diff * Lights[i].color;

        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = specStrength * spec * Lights[i].color;

        float distance = length(Lights[i].pos - FragPos);
        float att = clamp(1.0 - distance / Lights[i].radius, 0.0, 1.0);
        
        diffuse *= att;
        specular *= att;

        lighting += (diffuse + specular);
    }
    FragColor = vec4(lighting, alpha);
}