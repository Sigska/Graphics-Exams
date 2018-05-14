#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

//out vec2 TexCoords;
out vec4 pass;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
   // TexCoords = aTexCoords; 
	FragPos = vec3(model * vec4(aPos, 1.0));   
	pass = vec4(aPos, 1.0);		// pass to fragment for terrain colours
	Normal = aNormal;
	gl_Position = projection * view * vec4(FragPos, 1.0);
	
	
    
}