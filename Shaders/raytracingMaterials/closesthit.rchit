#version 460
#extension GL_GOOGLE_include_directive : enable
#include "common.glsl"
layout(location = 0) rayPayloadInEXT HitPayload payload;
layout(location = 1) rayPayloadInEXT ShadowPayload shadowPayload;

#include "vertex.glsl"
#include "calcLight.glsl"
#include "traceNextRay.glsl"

hitAttributeEXT vec3 attribs;

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
    //vec3 worldPos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
    vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(vertex.pos.xyz, 1.0));
    vec3 worldNormal = mat3(gl_ObjectToWorldEXT) * vertex.normal;

    //material
    uint32_t materialIndex = primMesh.materialIndex;
    Material material = materials[nonuniformEXT(materialIndex)];

    //lighting
    vec3 toLightDir = normalize(- ubo.lightDirection.xyz);
    vec3 lightColor = ubo.lightColor.xyz;
    float dotNL = dot(worldNormal, toLightDir);

    vec3 albedo = material.diffuse.xyz;
    if(material.textureIndex > - 1) {
        albedo = texture(textures[nonuniformEXT(material.textureIndex)], vertex.uv).xyz;
    }

    vec3 color = vec3(0, 0, 0);

    //lambert
    if(material.materialType == 0) {
        vec3 toEyeDir = normalize(ubo.cameraPosition.xyz - worldPos);
        color = LambertLight(worldNormal, toLightDir, albedo, lightColor, ubo.ambientColor.xyz);
        if(dotNL > 0) {
            color += PhongSpecular(worldNormal, - toLightDir, toEyeDir, material.specular);
        }
        //shadow
        if(ShootShadowRay(worldPos, toLightDir, 0)){
            color*=0.8;
        }
    }
    //metal
    if(material.materialType == 1) {
        color = Reflection(worldPos, worldNormal, gl_WorldRayDirectionEXT);
    }
    //glass
    if(material.materialType == 2) {
        color = Refraction(worldPos, worldNormal, gl_WorldRayDirectionEXT);
    }

    payload.hitValue = color;
}