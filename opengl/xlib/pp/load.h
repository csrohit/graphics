#ifndef LOAD_H
#define LOAD_H
#include <stdint.h>
#include <stddef.h>

struct Position
{
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

struct Texel
{
    float u;
    float v;
};

struct Face // triangle
{
    uint32_t vIndices[3];
    uint32_t nIndices[3];
    uint32_t tIndices[3];
    uint32_t fIndex;
};

struct Header
{
    uint32_t nVertices;
    uint32_t nIndices;
};

struct Group
{
    struct Group *next;
    int32_t       iMaterial;
    uint32_t      nFaces;
    uint32_t     *piFaces;
    char         *name;
};

struct Model
{
    struct Material *pMaterials;
    struct Position *pVertices;
    struct Position *pNormals;
    struct Texel    *pTexels;
    struct Group    *pGroups;
    struct Face     *pFaces;
    uint32_t         nVertices;
    uint32_t         nNormals;
    uint32_t         nTexels;
    uint32_t         nMaterials;
    uint32_t         nGroups;
    uint32_t         nFaces;
};

struct Material
{
    /*
     * @note:
     *  if metallic >  0 then illuminationModel = 3 and value of 
     *    metallic corresponds to ambient reflectance
     *  if metallic == 0 then illuminationModel = 2 and ambient reflectance is 1
     */
    /* Name of material */
    char *name;

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
    char *texturePath;
};

int        processMaterialFile(char *pFile, struct Model *pModel);
void       printMaterial(struct Material *pMaterial);
void       deleteMaterials(struct Material *pMaterials, int nMaterials);
int        loadModel(char *filename, struct Model **pModel);
void       unloadModel(struct Model *pModel);
void       printModel(struct Model *pModel);
static int findMaterial(struct Model *pModel, char *name);

#endif // !LOAD_H
