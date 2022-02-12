#version 460
#extension GL_EXT_ray_tracing : enable

struct RayPayload {
    vec3 color;
    float distance;
    vec3 normal;
    float reflector;
};
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

layout(set = 0, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 0, binding = 2) uniform UBO {
    mat4 viewInverse;
    mat4 projInverse;
    vec4 lightPos;
    int vertexSize;
}ubo;

layout(set = 0, binding = 3) buffer Vertices {
    vec4 v[];
}vertices;
layout(set = 0, binding = 4) buffer Indices {
    uint i[];
}indices;

hitAttributeEXT vec2 attribs;

struct Vertex {
    vec3 pos;
    vec3 normal;
    vec2 uv;
    vec4 color;
};

Vertex unpack(uint index) {
    //struct Vertexは16byte
    const int m = ubo.vertexSize / 16;
    vec4 d0 = vertices.v[m * index + 0];
    vec4 d1 = vertices.v[m * index + 1];
    vec4 d2 = vertices.v[m * index + 2];

    //Verticesはvec4の配列
    Vertex v;
    v.pos = d0.xyz;
    v.normal = vec3(d0.w, d1.x, d1.y);
    v.color = vec4(d2.x, d2.y, d2.z, 1.0);

    return v;
}

void main() {
    Vertex v0 = unpack(indices.i[3 * gl_PrimitiveID]);
    Vertex v1 = unpack(indices.i[3 * gl_PrimitiveID + 1]);
    Vertex v2 = unpack(indices.i[3 * gl_PrimitiveID + 2]);

    //normalを重心座標系で補間
    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    vec3 normal = normalize(v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z);

    //lighting
    vec3 lightVector = normalize(ubo.lightPos.xyz);
    float dot_product = max(dot(lightVector, normal), 0.6);
    rayPayload.color = v0.color.rgb * vec3(dot_product);

    rayPayload.distance = gl_RayTmaxEXT;
    rayPayload.normal = normal;
    rayPayload.reflector = ((v0.color.r == 1.0f) && (v0.color.g == 1.0f) && (v0.color.b == 1.0f)) ? 1.0f : 0.0f;
}