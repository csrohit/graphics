#ifndef MODEL_H
#include <stdint.h>

/**
 * @class Header
 * @brief Header of the Mode hex file
 *
 */
struct Header
{
    /**
     * @brief total number of vertices in the model
     */
    uint32_t nVertices;

    /**
     * @brief total number of indices in the model
     */
    uint32_t nIndices;
};

/**
 * @class Vertex
 * @brief Describes the properties of a vertex loaded from model
 */
struct Vertex
{
    /**
     * @brief X Co-ordinate of position
     */
    float x;
    /**
     * @brief Y Co-ordinate of position
     */
    float y;
    /**
     * @brief Z Co-ordinate of position
     */
    float z;
    /**
     * @brief U Co-ordinate of texture
     */
    float u;
    /**
     * @brief V Co-ordinate of texture
     */
    float v;
};

struct Position{
    float x;
    float y;
    float z;
};

struct Index
{
    int v;
    int t;
    int n;
};

struct Texture{
    float u;
    float v;
};

struct Triangle{
    struct Vertex a;
    struct Vertex b;
    struct Vertex c;
};

#endif // !MODEL_H
