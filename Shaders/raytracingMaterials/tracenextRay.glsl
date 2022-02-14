vec3 Reflection(vec3 worldPosition, vec3 worldNormal, vec3 incidentRay) {
    worldNormal = normalize(worldNormal);
    vec3 reflectedDir = reflect(incidentRay, worldNormal);

    float tmin = 0.001;
    float tmax = 10000.0;
    traceRayEXT(topLevelAS, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, worldPosition, tmin, reflectedDir, tmax, 0);
    return payload.hitValue;
}

vec3 Refraction(vec3 worldPosition, vec3 worldNormal, vec3 incidentRay) {
    worldNormal = normalize(worldNormal);
    const float refraction = 1.4;
    float direction = dot(worldNormal, incidentRay);
    vec3 refractedDir;
    vec3 normal;

    //air->glass
    if(direction < 0) {
        refractedDir = refract(incidentRay, worldNormal, 1.0 / refraction);
        normal = worldNormal;
    }else{
        refractedDir = refract(incidentRay, worldNormal, refraction);
        normal = -worldNormal;
    }

    //if not refraction
    if(length(refractedDir) < 0.01) {
        return Reflection(worldPosition, normal, incidentRay);
    }

    float tmin = 0.001;
    float tmax = 10000.0;
    traceRayEXT(topLevelAS, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, worldPosition, tmin, refractedDir, tmax, 0);
    return payload.hitValue;
}