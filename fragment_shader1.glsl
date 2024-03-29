#version 330

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess; //’shininess’ wp³ywa na rozproszenie/promieñ œwiat³a
                        //‘specular’
}; 
uniform Material material; 


in vec3 outcol;
in vec3 Normal;
in vec3 FragPos;  
out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;


void main()
{
    // ambient
    //float ambientStrength = 0.1;
    //vec3 ambient = ambientStrength * lightColor;
    vec3 ambient = lightColor * material.ambient;


    // diffuse 
    //vec3 norm = normalize(Normal);
    //vec3 lightDir = normalize(lightPos - FragPos);
    //float diff = max(dot(norm, lightDir), 0.0);
    //vec3 diffuse = diff * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * (diff * material.diffuse);


    // specular
    //float specularStrength = 0.5;
    //vec3 viewDir = normalize(viewPos - FragPos);
    //vec3 reflectDir = reflect(-lightDir, norm);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    //vec3 specular = specularStrength * spec * lightColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = lightColor * (spec * material.specular);
    
    vec3 result = (ambient + diffuse + specular);
    FragColor  = vec4(result, 1.0f);
    //gl_FragColor  = vec4(1.0f, 0.0f,0.0f, 1.0f);
} 
