#version 460
#extension GL_GOOGLE_include_directive : enable
#include "common.glsl"
layout(location = 1) rayPayloadInEXT ShadowPayload shadowPayload;

void main() {
    shadowPayload.isHit = false;
}