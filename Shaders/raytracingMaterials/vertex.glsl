struct Vertex {
    vec3 pos;
    vec3 normal;
    vec2 uv;
    vec4 color;
};

layout(buffer_reference, scalar)readonly buffer Indices {uvec3 i[];};
layout(buffer_reference, buffer_reference_align = 4, scalar)readonly buffer Vertices {Vertex v[];};


Vertex GetVertex(vec3 barycentricCoords, uint64_t indexBuffer, uint64_t vertexBuffer) {
    
    Indices indices = Indices(indexBuffer);
    Vertices vertices = Vertices(vertexBuffer);

    const uvec3 index = indices.i[gl_PrimitiveID];
    Vertex v0 = vertices.v[index.x];
    Vertex v1 = vertices.v[index.y];
    Vertex v2 = vertices.v[index.z];

    Vertex v = Vertex(vec3(0), vec3(0), vec2(0), vec4(0));

    //position
    v.pos += v0.pos * barycentricCoords.x;
    v.pos += v1.pos * barycentricCoords.y;
    v.pos += v2.pos * barycentricCoords.z;

    //Normal
    v.normal += v0.normal * barycentricCoords.x;
    v.normal += v1.normal * barycentricCoords.y;
    v.normal += v2.normal * barycentricCoords.z;

    //TexCoord
    v.uv += v0.uv * barycentricCoords.x;
    v.uv += v1.uv * barycentricCoords.y;
    v.uv += v2.uv * barycentricCoords.z;

    //color
    v.color += v0.color * barycentricCoords.x;
    v.color += v1.color * barycentricCoords.y;
    v.color += v2.color * barycentricCoords.z;

    return v;

}