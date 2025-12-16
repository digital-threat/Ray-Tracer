#version 460
#extension GL_EXT_ray_tracing: require
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require

#include "rt_input.glsl"
#include "rt_common.glsl"
#include "constants.glsl"

layout (location = 0) rayPayloadInEXT HitPayload inPayload;

void main()
{
    vec3 color = vec3(0);
    if (skyTextureId >= 0)
    {
        vec3 direction = normalize(gl_WorldRayDirectionEXT);

        vec2 texCoord;
        texCoord.x = 0.5f + atan(direction.z, direction.x) / K_2PI;
        texCoord.y = asin(direction.y) / K_PI + 0.5;

        color = texture(textures[nonuniformEXT (skyTextureId)], vec2(texCoord.x, 1 - texCoord.y)).rgb;
    }
    else
    {
        color = skyColor.rgb;
    }

    inPayload.color += inPayload.weight * color;

    inPayload.depth = 100;
    inPayload.done = 1;
}