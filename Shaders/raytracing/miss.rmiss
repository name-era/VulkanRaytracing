#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 hitValue;

//レイがどのオブジェクトにもヒットしなかったときに呼び出される
void main() {
    hitValue = vec3(0.0, 0.0, 0.2);
}