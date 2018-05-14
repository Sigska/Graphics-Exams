#version 330 core
out vec4 FragColor;

//in vec2 TexCoords;
in vec4 pass;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform vec3 lampPos;
uniform vec2 season;

void main()
{    
	
	//float lightStrength = 0.1;
	//vec3 terrainColor;
	//if(pass.y < -0.9) terrainColor = vec3(1.0, 1.0, 1.0);
	//else terrainColor = vec3(0.1, 0.5, 0.1);
	//if(pass.y > 0.0) terrainColor = vec3(0.0, 0.0, 1.0);
	//vec3 ambient = lightStrength * terrainColor;

	float lightStrength = 0.05;
	vec3 terrainColor;
	if(pass.y < season.x) terrainColor = vec3(0.0, 0.0, 1.0);
	else terrainColor = vec3(0.1, 0.5, 0.1);
	if(pass.y > season.y) terrainColor = vec3(1.0, 1.0, 1.0);
	vec3 ambient = lightStrength * vec3(1.0, 1.0, 1.0);

	// up to this point "ambient" is the objects ambient colour
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lampPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
	vec3 result = (ambient + diffuse) * terrainColor;
	FragColor = vec4(result, 1.0);
   	
}