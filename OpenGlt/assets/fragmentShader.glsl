#version 330 core

// OUTPUT
out vec4 FragColor;

// INPUT
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;


// STRUCTS
struct Material {
    // The vec3 colors are not used for lighting, but shininess is.
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirLight {
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
    float k0; // Constant attenuation
    float k1; // Linear attenuation
    float k2; // Quadratic attenuation
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;      // Cosine of the inner cutoff angle
    float outerCutOff; // Cosine of the outer cutoff angle
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float k0;
    float k1;
    float k2;
};

// UNIFORMS
#define MAX_POINT_LIGHT 20
#define MAX_SPOT_LIGHT 5

// Texture samplers
uniform sampler2D diffuse0;
uniform sampler2D specular0;

// Lights
uniform DirLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHT];
uniform SpotLight spotLights[MAX_SPOT_LIGHT];
uniform int noPointLights;
uniform int noSpotLights;

// Other
uniform Material material;
uniform vec3 viewPos;
uniform int useDebugColor;
uniform vec3 debugColor;

// FUNCTION PROTOTYPES
vec3 calcDirLight(vec3 norm, vec3 viewDir, vec3 diffMap, vec3 specMap);
vec3 calcPointLight(int idx, vec3 norm, vec3 viewDir, vec3 diffMap, vec3 specMap);
vec3 calcSpotLight(int idx, vec3 norm, vec3 viewDir, vec3 diffMap, vec3 specMap);

void main()
{
    // Pre-calculate vectors
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Use textures if available, otherwise fallback to material colors
    vec3 diffMap = vec3(texture(diffuse0, TexCoord));
    vec3 specMap = vec3(texture(specular0, TexCoord));
    if (diffMap == vec3(0.0)) {
        diffMap = material.diffuse;
    }
    if (specMap == vec3(0.0)) {
        specMap = material.specular;
    }

    // **CORRECTION**: Initialize result to zero to avoid garbage values.
    vec3 result = vec3(0.0);

    // Add directional light contribution
    result += calcDirLight(norm, viewDir, diffMap, specMap);

    // Add point light contributions
    for(int i = 0; i < noPointLights; i++)
    {
        result += calcPointLight(i, norm, viewDir, diffMap, specMap);
    }

    // Add spot light contributions
    for(int i = 0; i < noSpotLights; i++)
    {
        result += calcSpotLight(i, norm, viewDir, diffMap, specMap);
    }

    if (useDebugColor == 1) {
        FragColor = vec4(debugColor, 1.0);
    } else {
        FragColor = vec4(result, 1.0);
    }
}

// Directional Light Calculation
vec3 calcDirLight(vec3 norm, vec3 viewDir, vec3 diffMap, vec3 specMap)
{
    vec3 lightdir = normalize(-dirLight.direction);

    // Ambient
    vec3 ambient = dirLight.ambient * diffMap;

    // Diffuse
    float diff = max(dot(norm, lightdir), 0.0);
    vec3 diffuse = dirLight.diffuse * diff * diffMap;

    // Specular
    vec3 reflectDir = reflect(-lightdir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128);
    vec3 specular = dirLight.specular * spec * specMap;

    return (ambient + diffuse + specular);
}

// Point Light Calculation
vec3 calcPointLight(int idx, vec3 norm, vec3 viewDir, vec3 diffMap, vec3 specMap)
{
    vec3 lightDir = normalize(pointLights[idx].position - FragPos);

    // Ambient
    vec3 ambient = pointLights[idx].ambient * diffMap;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = pointLights[idx].diffuse * diff * diffMap;

    // Specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128);
    vec3 specular = pointLights[idx].specular * spec * specMap;

    // Attenuation
    float dist = length(pointLights[idx].position - FragPos);
    float attenuation = 1.0 / (pointLights[idx].k0 + pointLights[idx].k1 * dist + pointLights[idx].k2 * dist * dist);

    return (ambient + diffuse + specular) * attenuation;
}

// Spot Light Calculation
vec3 calcSpotLight(int idx, vec3 norm, vec3 viewDir, vec3 diffMap, vec3 specMap)
{
    vec3 lightDir = normalize(spotLights[idx].position - FragPos);
    float theta = dot(lightDir, normalize(-spotLights[idx].direction));

    // **CORRECTION**: Use the ambient color from the indexed spotlight.
    vec3 ambient = spotLights[idx].ambient * diffMap;

    // Check if the fragment is outside the spotlight's outer cone.
    // If so, only the ambient light (affected by attenuation) contributes.
    if(theta > spotLights[idx].outerCutOff)
    {
        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = spotLights[idx].diffuse * diff * diffMap;

        // Specular
        // **CORRECTION**: Use the specular color from the indexed spotlight.
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128);
        vec3 specular = spotLights[idx].specular * spec * specMap;

        // Calculate smooth intensity falloff between inner and outer cone
        // **CORRECTION**: Use cutoff values from the indexed spotlight.
        float intensity = (theta - spotLights[idx].outerCutOff) / (spotLights[idx].cutOff - spotLights[idx].outerCutOff);
        intensity = clamp(intensity, 0.0, 1.0);
        diffuse  *= intensity;
        specular *= intensity;

        // Attenuation
        // **CORRECTION**: Use attenuation values from the indexed spotlight.
        float dist = length(spotLights[idx].position - FragPos);
        float attenuation = 1.0 / (spotLights[idx].k0 + spotLights[idx].k1 * dist + spotLights[idx].k2 * dist * dist);

        return (ambient + diffuse + specular) * attenuation;
    }

    // Outside the cone, only return ambient light (without attenuation for a simpler effect, or you can apply it too)
    return vec3(0.0); // Or return 'ambient' if you want ambient light outside the cone
}
