#version 460
#extension GL_GOOGLE_include_directive : enable
#include "common.glsl"
layout(location = 0) rayPayloadInEXT Payload payload;

void main() {
    const vec3 worldRayDirection = gl_WorldRayDirectionEXT;
    payload.hitValue = texture(backGround, worldRayDirection).xyz;
}