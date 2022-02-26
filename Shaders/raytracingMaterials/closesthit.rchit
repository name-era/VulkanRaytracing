#version 460

#extension GL_GOOGLE_include_directive : enable
#include "common.glsl"
layout(location = 0) rayPayloadInEXT Payload payload;

#include "vertex.glsl"
#include "calcLight.glsl"
#include "traceNextRay.glsl"

hitAttributeEXT vec2 attribs;

void main() {
    
 
    payload.recursive = payload.recursive - 1;
    if(payload.recursive < 0) {
        payload.hitValue = vec3(0, 0, 0);
        return;
    }

    const vec3 barycentricCoords = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    PrimMesh primMesh = primMeshes[gl_InstanceID];

    //vertex
    Vertex vertex = GetVertex(barycentricCoords, primMesh.indexBuffer, primMesh.vertexBuffer);
    vec3 worldPosition = gl_ObjectToWorldEXT * vec4(vertex.pos, 1);
    vec3 worldNormal = normalize(mat3(gl_ObjectToWorldEXT) * vertex.normal);

    //material
    uint32_t materialIndex = primMesh.materialIndex;
    Material material = materials[nonuniformEXT(materialIndex)];

    //lighting
    vec3 tolightDir = normalize(- ubo.lightPos.xyz);
    vec3 lightColor = ubo.lightColor.xyz;
    float dotNL = dot(tolightDir, worldNormal);

    vec3 albedo = material.diffuse.xyz;
    if(material.textureIndex > - 1) {
        albedo = texture(textures[nonuniformEXT(material.textureIndex)], vertex.uv).xyz;
    }

    vec3 color = vec3(0, 0, 0);
    //lambert
    if(material.materialType == 0) {
        vec3 toEyeDir = normalize(ubo.cameraPosition.xyz - worldPosition);
        color = LambertLight(worldNormal, tolightDir, albedo, lightColor, ubo.ambientColor.xyz);
        if(dotNL > 0) {
            color += PhongSpecular(worldNormal, - tolightDir, toEyeDir, material.specular);
        }
    }
    //metal
    if(material.materialType == 1) {
        color = Reflection(worldPosition, worldNormal, gl_WorldRayDirectionEXT);
    }
    //glass
    if(material.materialType == 2) {
        color = Refraction(worldPosition, worldNormal, gl_WorldRayDirectionEXT);
    }

    payload.hitValue = color;
}