#ifndef LOAD_MODEL_H
#define LOAD_MODEL_H
#include <stdint.h>

typedef struct Position
{
    float x;
    float y;
    float z;
} Position_t;

typedef struct Texel
{
    float u;
    float v;
} Texel_t;

typedef struct Normal
{
    float x;
    float y;
    float z;
} Normal_t;

typedef struct Vertex
{
    Position_t position;
    Normal_t   normal;
    // Texel_t    Texel;

} Vertex_t;

typedef struct Header
{
    uint32_t nVertices;
    uint32_t nIndices;
} Header_t;

typedef struct Body
{
    Vertex_t* pVertices;
    uint32_t* pIndices;
} Body_t;

typedef struct Model
{
    Header_t header;
    Body_t  body;
} Model_t;

Model_t* loadModel(const char* pFilename);
int      unloadModel(Model_t* pModel);

#endif // LOAD_MODEL_H