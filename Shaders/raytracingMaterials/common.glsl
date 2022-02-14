#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_buffer_refelence : enable
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

struct Payload {
    vec3 hitValue;
    int recursive;
};

struct PrimMeshInfo {
    uint64_t indexBuffer;
    uint64_t vertexBuffer;
    uint32_t materialIndex;
};

struct MaterialInfo {
    vec4 diffuse;
    vec4 specular;
    int32_t materialKind;
    int32_t textureIndex;
};

layout(binding = BIND_TLAS, set = 0) uniform accelerationStructureEXT topLevelAS;
layout(binding = BIND_IMAGE, set = 0, rgba8) uniform image2D image;
layout(binding = BIND_SCENEPARAM, set = 0) uniform UBO {
    mat4 viewInverse;
    mat4 projInverse;
    vec4 lightPos;
    vec4 lightColor;
    vec4 ambientColor;
} ubo;
layout(binding = BIND_BACKGROUND, set = 0) uniform samplerCube backGround;
layout(binding = BIND_OBJECTLIST, set = 0) readonly buffer primMeshInfo {PrimMeshInfo i[];};
layout(binding = BIND_MATERIALLIST, set = 0) readonly buffer materials {MaterialInfo m[];};
layout(binding = BIND_TEXTURELIST, set = 0) uniform sampler2D textures[];

layout(constant_id = 0) const int MAX_RECURSION = 0;
layout(location = 0) rayPayloadInEXT Payload payload;