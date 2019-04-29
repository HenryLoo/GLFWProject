#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // Attenuation
    float constant;
    float linear;
    float quadratic;
};

out vec4 fragColour;

in vec3 normal;
in vec2 texCoord;
in vec3 fragPos;

uniform Material material;
uniform DirectionalLight dirLight;
uniform PointLight pointLight;
uniform vec3 viewPos;

vec3 calculateDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);

    // Ambient component.
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));

    // Diffuse component.
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord));

    // Specular component.
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord));

    return (ambient + diffuse + specular);
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // Ambient component.
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));

    // Diffuse component.
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord));

    // Specular component.
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord));

    // Calculate attenuation.
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    return attenuation * (ambient + diffuse + specular);
}

void main()
{
    float alpha = texture(material.diffuse, texCoord).a;
    if (alpha < 0.1) discard;

    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);

    // Calculate directional lighting.
    vec3 result = calculateDirLight(dirLight, norm, viewDir);

    // Calculate point lighting.
    result += calculatePointLight(pointLight, norm, fragPos, viewDir);

    fragColour = vec4(result, alpha);
}
