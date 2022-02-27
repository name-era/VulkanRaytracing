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

struct Payload {
    vec3 hitValue;
    int recursive;
};

struct PrimMesh {
    uint64_t indexBuffer;
    uint64_t vertexBuffer;
    uint32_t materialIndex;
    //メモリアラインメントを揃える4+8byte
    int32_t padding0;
    int64_t padding1;
};

struct Material {
    vec4 diffuse;
    vec4 specular;
    int32_t materialType;
    int32_t textureIndex;
    //メモリアラインメントを揃える4×2byte
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
    vec3 cameraPosition;
} ubo;

layout(binding = BIND_BACKGROUND, set = 0) uniform samplerCube backGround;
layout(binding = BIND_OBJECTLIST, set = 0) readonly buffer _PrimMesh {PrimMesh primMeshes[]; };
layout(binding = BIND_MATERIALLIST, set = 0) readonly buffer _Material {Material materials[]; };
layout(binding = BIND_TEXTURELIST, set = 0) uniform sampler2D textures[];