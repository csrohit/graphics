#ifndef LOAD_H
#define LOAD_H
#include <stdint.h>
#include <stddef.h>

typedef struct Position
{
    float x;
    float y;
    float z;
} Position;

typedef struct Index
{
    int v;
    int t;
    int n;
} Index;

typedef struct Texel
{
    float u;
    float v;
} Texel;

struct Face // triangle
{
    uint32_t vIndices[3];
    uint32_t nIndices[3];
    uint32_t tIndices[3];
    uint32_t fIndex;
};

typedef struct Header
{
    char     name[20];
    uint32_t nVertices;
    uint32_t nIndices;
} Header;

struct Group
{
    struct Group* next;
    int32_t       iMaterial;
    uint32_t      nFaces;
    uint32_t*     piFaces;
    char*         name;
};

typedef struct Vertex
{
    Position position;
    Position normal;
    Texel    texel;
} Vertex;

typedef struct Model
{
    Header    header;
    Vertex*   pVertices;
    uint32_t* pIndices;
} Model;

struct Material
{
    /*
     * @note:
     *  if metallic >  0 then illuminationModel = 3 and value of
     *    metallic corresponds to ambient reflectance
     *  if metallic == 0 then illuminationModel = 2 and ambient reflectance is 1
     */
    /* Name of material */
    char* name;

    /* [Metallic] - Ambient reflectance of material */
    float rAmbient[4];

    /* [BaseColor] - Diffuse reflectance of material */
    float rDiffuse[4];

    /* [Specular] - Specular reflectance of material */
    float rSpecular[4];

    /* [Emission] - Color emitted by material */
    float emmission[4];

    /* [Roughness] - focus of specular hightlight of material */
    float shininess;

    /* [IOR] Refractive index */
    float opeticalDensity;

    /* [Alpha] - dissolveFactor of material */
    float dissolveFactor;

    /* [Metalic] - Model of illumination */
    uint32_t illuminationModel;

    /* file path of texture image */
    char* texturePath;
};

int        processMaterialFile(char* pFile, struct Model* pModel);
void       printMaterial(struct Material* pMaterial);
void       deleteMaterials(struct Material* pMaterials, int nMaterials);
int        readObj(char* filename, Model* pModel);
void       unloadModel(struct Model* pModel);
void       printModel(struct Model* pModel);
static int findMaterial(struct Model* pModel, char* name);
int        exportModel(Model* pModel, const char* pFileName);
int        loadModel(Model* pModel, const char* pFileName);

#endif // !LOAD_H
