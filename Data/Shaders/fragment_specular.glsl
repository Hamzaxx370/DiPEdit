#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;

struct LightPoint
{
	vec3 pos;
	vec3 dir;
	vec3 color;
	float intensity;
};

uniform sampler2D tex0;
uniform LightPoint Lights[2];
uniform int LightCount;
uniform mat4 view;

void main()
{
	vec4 TexColor = texture(tex0,UV);
	vec3 ObjectColor = TexColor.rgb;
    float alpha = TexColor.a;
    float specalpha = alpha;    

    if (alpha == 0)
        discard;
    else
        alpha = 1.0f;

	vec3 Lighting = ObjectColor;
	for (int i = 0; i < LightCount; i++)
	{
		vec3 LightPos = Lights[i].pos;
		vec3 LightColor = Lights[i].color;
		vec3 LightDir = Lights[i].dir;
		vec3 ViewPos = (inverse(view))[3].xyz;
		vec3 Norm = normalize(Normal);
		vec3 LightDirNorm = normalize(LightPos - FragPos);
		float diff = max(dot(Norm,LightDirNorm),0.0);
		vec3 ViewDir = normalize(ViewPos - FragPos);
		vec3 ReflectDir =  reflect(-LightDirNorm,Norm);
		float Spec = pow(max(dot(ViewDir,ReflectDir),0.0),32.0);
		float Theta = dot(-LightDirNorm,normalize(-LightDir));
		float Intensity = Lights[0].intensity;
		vec3 Ambient = 0.1 * LightColor * ObjectColor;
		vec3 Diffuse = diff * LightColor * ObjectColor;
		vec3 Specular = Spec * LightColor * 0.05 * specalpha ;
		Lighting += (Ambient + Diffuse + Specular) * Intensity;
	}
	FragColor = vec4(ObjectColor,alpha) * vec4(Lighting,1.0f);
}