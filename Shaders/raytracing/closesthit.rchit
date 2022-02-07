#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) rayPayloadInEXT vec3 hitValue;
hitAttributeEXT vec2 attribs;

//レイがヒットしたときに呼び出される
void main() {
    //重心座標系（現在処理している三角形の頂点をどの割合で混ぜるか）
    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    hitValue = barycentricCoords;
}