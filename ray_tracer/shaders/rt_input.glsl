#define MAX_LIGHTS 8

#include "lighting.glsl"

layout (push_constant) uniform PushConstants
{
    uint samplesPerPixel;
    uint frame;
};

layout (set = 0, binding = 0) uniform accelerationStructureEXT tlas;
layout (set = 0, binding = 1, rgba32f) uniform image2D renderTarget;

layout (set = 1, binding = 0) uniform Scene
{
    mat4 matrixV;
    mat4 matrixP;
    mat4 matrixIV;
    mat4 matrixIP;
    mat4 matrixVP;
    mat4 mainLightVP;
    vec4 mainLightDir;
    vec4 mainLightColor;
    vec4 skyColor;
    vec4 cameraPos;
    LightData lightBuffer[MAX_LIGHTS];
    uint lightCount;
    int skyTextureId;
};

layout (set = 1, binding = 1, scalar) readonly buffer ObjectData_
{
    ObjectData i[];
} objData;

layout (set = 1, binding = 2) uniform sampler2D[] textures;

Light GetLight(uint index, vec3 position)
{
    vec3 lightColor = lightBuffer[index].color.rgb;
    float lightType = lightBuffer[index].color.w;
    vec3 lightPosition = lightBuffer[index].position.xyz;
    vec3 lightSpotDirection = lightBuffer[index].spotDirection.xyz;
    float lightSpotCutoff = lightBuffer[index].spotDirection.w;

    Light light;
    light.color = lightColor;

    if (lightType == 0.0f) // Directional
    {
        light.direction = normalize(lightPosition);
        light.attenuation = 1.0f;
    }
    else if (lightType == 1.0f) // Point
    {
        light.direction = normalize(lightPosition - position);
        float dist = length(lightPosition - position);
        light.attenuation = 1.0 / (1.0f + 0.7f * dist + 1.8f * (dist * dist));
    }
    else if (lightType == 2.0f) // Spot
    {
        light.direction = normalize(lightPosition - position);
        float SdotL = dot(lightSpotDirection, normalize(-light.direction));
        light.attenuation = SdotL > lightSpotCutoff ? 1.0f : 0.0f;
    }

    return light;
}
