#include "static_geometry.h"

#include "batch.h"

static std::vector<Vertex> unitQuadVerts = {
    {glm::vec3(-1.0,  1.0, 0.0), glm::vec3(1.0, 0.0, 0.0),  glm::vec2(0.0, 1.0)},
    {glm::vec3(1.0,  1.0, 0.0), glm::vec3(1.0, 0.0, 0.0),  glm::vec2(1.0, 1.0)},
    {glm::vec3(1.0, -1.0, 0.0), glm::vec3(1.0, 0.0, 0.0),  glm::vec2(1.0, 0.0)},
    {glm::vec3(-1.0, -1.0, 0.0), glm::vec3(1.0, 0.0, 0.0),  glm::vec2(0.0, 0.0)},
};

static std::vector<uint32_t> unitQuadIndices = {
    0, 1, 2,
    2, 3, 0
};

static Batch unitQuadBatch;

void InitStaticGeometry() {
    unitQuadBatch = Batch(4, 6);
    unitQuadBatch.Add(&unitQuadVerts[0], unitQuadVerts.size(), &unitQuadIndices[0], unitQuadIndices.size());
}

void DestroyStaticGeometry() {
    unitQuadBatch.Kill();
}

Batch& GetUnitQuadBatch() {
    return unitQuadBatch;
}
