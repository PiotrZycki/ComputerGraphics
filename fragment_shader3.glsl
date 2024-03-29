#version 330

uniform vec3 objectColor;
in vec3 outcol;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

void main()
{
    //FragColor  = vec4(outcol, 1.0f);
    FragColor  = vec4(objectColor, 1.0f);
} 
