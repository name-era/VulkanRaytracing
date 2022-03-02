#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types : enable
#extension GL_EXT_nonuniform_qualifier : enable

#define BIND_TLAS           (0)
#define BIND_IMAGE          (1)
#define BIND_SCENEPARAM     (2)
#define BIND_BACKGROUND     (3)
#define BIND_OBJECTLIST     (4)
#define BIND_MATERIALLIST   (5)
#define BIND_TEXTURELIST    (6)

struct HitPayload {
    vec3 hitValue;
    int recursive;
};
struct ShadowPayload{
    bool isHit;
};

struct PrimMesh {
    uint64_t indexBuffer;
    uint64_t vertexBuffer;
    uint32_t materialIndex;
    //memory alignment 4+8byte
    int32_t useShadow;
    int64_t padding1;
};

struct Material {
    vec4 diffuse;
    vec4 specular;
    int32_t materialType;
    int32_t textureIndex;
    //memory alignment 4×2byte
    int64_t padding0;
};

layout(binding = BIND_TLAS, set = 0) uniform accelerationStructureEXT topLevelAS;
layout(binding = BIND_IMAGE, set = 0, rgba8) uniform image2D image;
layout(binding = BIND_SCENEPARAM, set = 0) uniform UBO {
    mat4 viewInverse;
    mat4 projInverse;
    vec4 lightDirection;
    vec4 lightColor;
    vec4 ambientColor;
    vec4 cameraPosition;
    vec4 pointLightPosition;
    int shaderFlags;
} ubo;

layout(binding = BIND_BACKGROUND, set = 0) uniform samplerCube backGround;
layout(binding = BIND_OBJECTLIST, set = 0) readonly buffer _PrimMesh {PrimMesh primMeshes[]; };
layout(binding = BIND_MATERIALLIST, set = 0) readonly buffer _Material {Material materials[]; };
layout(binding = BIND_TEXTURELIST, set = 0) uniform sampler2D textures[];

uint randomU(vec2 uv)
{
    float r = dot(uv, vec2(127.1, 311.7));
    return uint(12345 * fract(sin(r) * 43758.5453123));
}


float nextRand(inout uint s) {
    s = (1664525u * s + 1013904223u);
    return float(s & 0x00FFFFFF) / float(0x01000000);
}

mat3 angleAxis3x3(float angle, vec3 axis) {
    float c, s;
    s = sin(angle);
    c = cos(angle);

    float t = 1 - c;
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    return mat3(
            t * x * x + c, t * x * y - s * z, t * x * z + s * y,
            t * x * y + s * z, t * y * y + c, t * y * z - s * x,
            t * x * z - s * y, t * y * z + s * x, t * z * z + c
        );
}

vec3 getConeSample(inout uint randSeed, vec3 direction, float coneAngle){
    float cosAngle=cos(coneAngle);
    const float PI=3.1415926535;

    //cos～1
    float z = nextRand(randSeed) * (1.0f - cosAngle) + cosAngle;
    //0～2π
    float phi = nextRand(randSeed) * 2.0f * PI;
    float x = sqrt(1.0 - z * z) * cos(phi);
    float y = sqrt(1.0 - z * z) * sin(phi);
    vec3 north = vec3(0.f, 0.f, 1.f);
    vec3 axis = normalize(cross(normalize(direction), north));
    float angle = acos(dot(normalize(direction), north));
    mat3 R = angleAxis3x3(angle, axis);
    return R * vec3(x, y, z);
}