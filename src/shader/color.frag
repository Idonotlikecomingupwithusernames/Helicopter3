#version 330 core

out vec4 FragColor;

in vec3 tNormal;
in vec3 tFragPos;
in vec2 tUV;

struct Material
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 emission;
    float shininess;
};

struct DirLight 
{
    vec3 direction;
    vec4 color;
};

struct SpotLight 
{
    vec3 position;
    vec3 direction;
    vec4 color;

    float constant;
    float linear;
    float quadratic; 
    float phi;
};

uniform Material uMaterial;
uniform vec3 uCamPos;

uniform DirLight uDirLight;
uniform SpotLight uSpotLight[4];
uniform vec3 uAmbient;

uniform sampler2D texAO;
uniform sampler2D texDIF;
uniform sampler2D texSPEC;
uniform sampler2D texEM;

vec3 bPDir(vec3 lightColor, vec3 lightDir, vec3 viewDir, vec3 normal, Material material) {
    /* Calculate diffuse */
    vec3 diffuse = texture(texDIF, tUV).rgb * lightColor * (max(dot(normal, lightDir), 0.0) * material.diffuse.rgb);
    /* Calculate specular */
    vec3 halfDir = normalize(viewDir + lightDir);
    float specDot = max(dot(halfDir, normal), 0.0);
    vec3 specular = texture(texSPEC, tUV).rgb * lightColor * pow(specDot, material.shininess) * material.specular.rgb;

    return diffuse + specular;
}

vec3 bPSpot(SpotLight spotLight, vec3 viewDir, vec3 normal, vec3 fragPos, Material material) {
    vec3 lightDir = normalize(spotLight.position - fragPos);
    float theta = dot(lightDir, normalize(-spotLight.direction));

    vec3 lightSum = vec3(0.0, 0.0, 0.0);

    if(theta > spotLight.phi) 
    {
        /* Calculate diffuse */
        vec3 diffuse = texture(texDIF, tUV).rgb * spotLight.color.rgb * (max(dot(normal, lightDir), 0.0) * material.diffuse.rgb);
        /* Calculate specular */
        vec3 halfDir = normalize(viewDir + lightDir);
        float specDot = max(dot(halfDir, normal), 0.0);
        vec3 specular = texture(texSPEC, tUV).rgb * spotLight.color.rgb * pow(specDot, material.shininess) * material.specular.rgb;
        /* Calculate attenuation */
        float distance = length(spotLight.position - fragPos);
        float attenuation = 1.0 / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * (distance * distance));

        lightSum += (diffuse + specular) * attenuation;
    }
    
    return lightSum;
} 

void main(void)
{
    vec3 lightSum = vec3(0.0);
    vec3 sptlts = vec3(0.0);

    vec3 viewDir = normalize(uCamPos - tFragPos);
    vec3 n = normalize(tNormal);

    /* Calculate ambient */
    vec3 ambient = texture(texAO, tUV).rgb * uAmbient * uMaterial.ambient.rgb;

    /* Calculate directional light */
    vec3 dLightDir = normalize(-uDirLight.direction);

    lightSum += bPDir(uDirLight.color.rgb, dLightDir, viewDir, n, uMaterial);

    /* Calculate spotlights */
    for(int i = 0; i < 4; i++){
        sptlts += bPSpot(uSpotLight[i], viewDir, n, tFragPos, uMaterial);
    }

    lightSum += ambient + sptlts + texture(texEM, tUV).rgb * uMaterial.emission.rgb;
    FragColor = vec4(lightSum, 1.0);
}