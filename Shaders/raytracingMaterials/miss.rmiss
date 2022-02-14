#version 460
#extension GL_EXT_ray_tracing : enable

#include "common.glsl"
layout(location = 0) rayPayloadInEXT Payload payload;

void main() {
    const vec3 worldRayDirection = normalize(gl_WorldRayDirectionEXT);
    payload.hitValue = texture(backGround, worldRayDirection).xyz;
}