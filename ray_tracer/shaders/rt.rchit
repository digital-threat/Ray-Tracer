#version 460
#extension GL_EXT_ray_tracing: require
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require
#extension GL_EXT_buffer_reference2: require

#include "rt_input.glsl"
#include "rt_common.glsl"
#include "sampling.glsl"

layout (location = 0) rayPayloadInEXT HitPayload inPayload;
layout (location = 1) rayPayloadEXT bool isInShadow;

hitAttributeEXT vec2 attributes;

layout (buffer_reference, scalar) buffer VertexBuffer
{
    Vertex vertices[];
};

layout (buffer_reference, scalar) buffer IndexBuffer
{
    ivec3 indices[];
};

layout (buffer_reference, scalar) buffer MaterialBuffer
{
    Material materials[];
};

layout (buffer_reference, scalar) buffer MatIdBuffer
{
    int indices[];
};

void main()
{
    ObjectData data = objData.i[gl_InstanceCustomIndexEXT];
    VertexBuffer vertexBuffer = VertexBuffer(data.vertexAddress);
    IndexBuffer indexBuffer = IndexBuffer(data.indexAddress);
    MaterialBuffer materialBuffer = MaterialBuffer(data.materialAddress);
    MatIdBuffer matIdBuffer = MatIdBuffer(data.matIdAddress);

    ivec3 primitive = indexBuffer.indices[gl_PrimitiveID];

    Vertex v0 = vertexBuffer.vertices[primitive.x];
    Vertex v1 = vertexBuffer.vertices[primitive.y];
    Vertex v2 = vertexBuffer.vertices[primitive.z];

    const vec3 barycentrics = vec3(1.0 - attributes.x - attributes.y, attributes.x, attributes.y);

    const vec3 position = v0.position * barycentrics.x + v1.position * barycentrics.y + v2.position * barycentrics.z;
    const vec3 positionWS = vec3(gl_ObjectToWorldEXT * vec4(position, 1.0));

    const vec3 normal = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;
    const vec3 normalWS = normalize(vec3(normal * gl_WorldToObjectEXT));

    const vec2 uv = v0.uv * barycentrics.x + v1.uv * barycentrics.y + v2.uv * barycentrics.z;

    int matId = matIdBuffer.indices[gl_PrimitiveID];
    Material material = materialBuffer.materials[matId];

    // Pathtracing

    inPayload.color += inPayload.weight * material.emission;

    vec3 tangent, bitangent;
    createCoordinateSystem(normalWS, tangent, bitangent);
    vec3 rayOrigin = positionWS;// + normalWS * 1e-4;
    vec3 rayDirection = samplingHemisphere(inPayload.seed, tangent, bitangent, normalWS);

    const float cos_theta = dot(rayDirection, normalWS);
    const float p = cos_theta / M_PI;

    vec3 diffuse = material.diffuse;
    if (material.diffuseTextureId >= 0)
    {
        uint index = data.txtOffset + material.diffuseTextureId;
        diffuse *= texture(textures[nonuniformEXT (index)], vec2(uv.x, 1 - uv.y)).xyz;
    }

    vec3 BRDF = diffuse / M_PI;

    inPayload.rayOrigin = rayOrigin;
    inPayload.rayDirection = rayDirection;
    inPayload.weight *= BRDF * cos_theta / p;

//    Light mainLight;
//    mainLight.color = mainLightColor.xyz;
//    mainLight.direction = normalize(mainLightDir.xyz);
//    mainLight.attenuation = 1.0f;

//    float lightIntensity = mainLightColor.w;

//    vec3 diffuse = ComputeDiffuse(material, mainLight.direction, normalWS);
//    if (material.diffuseTextureId >= 0)
//    {
//        uint index = data.txtOffset + material.diffuseTextureId;
//        diffuse *= texture(textures[nonuniformEXT (index)], vec2(uv.x, 1 - uv.y)).xyz;
//    }
//
//    vec3 specular = vec3(0);
//    float attenuation = 0.3f;
//    if (dot(normal, mainLight.direction) > 0)
//    {
//        float tMin = 0.001f;
//        float tMax = 10000.0f;
//        vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
//        vec3 rayDir = mainLight.direction;
//        uint flags = gl_RayFlagsSkipClosestHitShaderEXT;
//        isInShadow = true;
//        traceRayEXT(tlas, // acceleration structure
//                    flags, // rayFlags
//                    0xFF, // cullMask
//                    0, // sbtRecordOffset
//                    0, // sbtRecordStride
//                    1, // missIndex
//                    origin, // ray origin
//                    tMin, // ray min range
//                    rayDir, // ray direction
//                    tMax, // ray max range
//                    1 // payload (location = 1)
//        );
//
//        if (!isInShadow)
//        {
//            attenuation = 1.0f;
//
//            specular = ComputeSpecular(material, mainLight.direction, normalWS, gl_WorldRayDirectionEXT);
//            if (material.specularTextureId >= 0)
//            {
//                uint index = data.txtOffset + material.specularTextureId;
//                specular *= texture(textures[nonuniformEXT (index)], vec2(uv.x, 1 - uv.y)).xyz;
//            }
//        }
//    }
//
//    if (material.illum == 3)
//    {
//        vec3 origin = positionWS;
//        vec3 direction = reflect(gl_WorldRayDirectionEXT, normal);
//        inPayload.attenuation *= material.specular;
//        inPayload.done = 0;
//        inPayload.rayOrigin = origin;
//        inPayload.rayDirection = direction;
//    }
//
//    inPayload.color = vec3(attenuation * lightIntensity * (diffuse + specular));
}
