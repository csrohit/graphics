#include "load.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef EXPORT
int main(int argc, char* argv[])
{
    if (3 != argc)
    {
        fprintf(stderr, "Please provide input and output mmodels\n");
        return -1;
    }

    struct Model model = {};
    int          rc    = readObj(argv[1], &model);
    if (rc != 0)
    {
        fprintf(stderr, "failed to load model\n");
        return (-1);
    }

    // printModel(pModel);
    exportModel(&model, argv[2]);

    unloadModel(&model);

    return (0);
}

#endif

void printModel(struct Model* pModel)
{
    if (NULL != pModel)
    {
        fprintf(stdout, "Name: %s\n", pModel->header.name);
        fprintf(stdout, "nVertices: %u\n", pModel->header.nVertices);
        fprintf(stdout, "nIndices: %u\n", pModel->header.nIndices);

        if (NULL != pModel->pVertices)
        {
            for (uint32_t idx = 0; idx < pModel->header.nVertices; idx++)
            {
                fprintf(
                    stdout, "Vertex %d: %5.2f %5.2f %5.2f\t %5.2f %5.2f\t %5.2f %5.2f %5.2f\n", idx, pModel->pVertices[idx].position.x, pModel->pVertices[idx].position.y,
                    pModel->pVertices[idx].position.z, pModel->pVertices[idx].texel.u, pModel->pVertices[idx].texel.v, pModel->pVertices[idx].normal.x, pModel->pVertices[idx].normal.y,
                    pModel->pVertices[idx].normal.z);
            }

            for (uint32_t idx = 0; idx < pModel->header.nIndices; idx++)
            {
                fprintf(stdout, "Index %d, vertex %d\n", idx, pModel->pIndices[idx]);
            }
        }
    }
}

/**
 * @brief Find group with matching name
 *
 * @param pModel [in] - Pointer to model
 * @param name   [in] - name of model
 *
 * @returns pointer to group else NULL
 */
// struct Group *findGroup(struct Model *pModel, char *name)
// {
//     struct Group *pGroup = pModel->pGroups;
//
//     while (NULL != pGroup)
//     {
//         if (0 == strcmp(pGroup->name, name))
//         {
//             break;
//         }
//         pGroup = pGroup->next;
//     }
//     return pGroup;
// }
//
// struct Group *addGroup(struct Model *pModel, char *name)
// {
//     struct Group *pGroup = findGroup(pModel, name);
//     if (NULL == pGroup)
//     {
//         pGroup            = (struct Group *)malloc(sizeof(Group));
//         pGroup->name      = strdup(name);
//         pGroup->nFaces    = 0U;
//         pGroup->piFaces   = NULL;
//         pGroup->iMaterial = -1;
//         pGroup->next      = pModel->pGroups;
//         pModel->pGroups   = pGroup;
//         ++pModel->nGroups;
//     }
//
//     return pGroup;
// }

int readObj(char* filename, Model* pModel)
{
    char  buff[128] = {0};
    FILE* pFile     = NULL;

    struct Position* pPositions    = NULL;
    struct Position* pNormals      = NULL;
    struct Texel*    pTexels       = NULL;
    struct Index*    pInputIndices = NULL;

    uint32_t nInputIndices = 0U;
    uint32_t nPositions    = 0U;
    uint32_t nNormals      = 0U;
    uint32_t nTexels       = 0U;

    pFile = fopen(filename, "r");
    if (NULL == pFile)
    {
        fprintf(stderr, "Failed to open model \"%s\"\n", filename);
        return (-1);
    }

    /* Calculate nVertices, nNormals, nTexels, nIndices */
    while (EOF != fscanf(pFile, "%s", buff))
    {
        switch (buff[0])
        {
            case 'o':
            {
                fgets(buff, sizeof(buff), pFile);
                sscanf(buff, "%s", pModel->header.name);
                fprintf(stdout, "Model Name: %s\n", pModel->header.name);
                break;
            }
            case 'm':
            {
                fscanf(pFile, "%s", buff);
                processMaterialFile(buff, pModel);
                break;
            }
            case 'f':
            {
                int v, n, t;

                fgets(buff, sizeof(buff), pFile);
                nInputIndices += 3;
                break;
            }
            case 'v':
            {
                switch (buff[1])
                {
                    case '\0':
                    {
                        ++nPositions;
                        break;
                    }
                    case 'n':
                    {
                        ++nNormals;
                        break;
                    }
                    case 't':
                    {
                        ++nTexels;
                        break;
                    }
                }
                fgets(buff, sizeof(buff), pFile);
                break;
            }
            case '#':
            {
                // comment
            }
            default:
            {
                // read and dump content
                fgets(buff, sizeof(buff), pFile);
                break;
            }
        }
    }

    fprintf(stdout, "nPositions %d, nNormals %d, nTexels %d, nInputIndices %d\n", nPositions, nNormals, nTexels, nInputIndices);

    (void)fseek(pFile, 0L, SEEK_SET);
    pPositions    = (struct Position*)malloc(sizeof(struct Position) * nPositions);
    pNormals      = (struct Position*)malloc(sizeof(struct Position) * nNormals);
    pTexels       = (struct Texel*)malloc(sizeof(struct Texel) * nTexels);
    pInputIndices = (struct Index*)malloc(sizeof(struct Index) * nInputIndices);

    struct Position* currentPosition = pPositions;
    struct Position* currentNormal   = pNormals;
    struct Texel*    currentTexel    = pTexels;
    struct Index*    currentIndex    = pInputIndices;

    /* Load vertices data */
    while (EOF != fscanf(pFile, "%s", buff))
    {
        switch (buff[0])
        {
            case 'm':
            {
                fgets(buff, sizeof(buff), pFile);
                sscanf(buff, "%s", buff);
                break;
            }
            case 'g':
            {
            }
            case 'u':
            {
                fgets(buff, sizeof(buff), pFile);
                fprintf(stderr, "Group is not supported %s\n", buff);
                break;
            }
            case 'f':
            {
                int v = 0;
                int t = 0;
                int n = 0;
                fscanf(pFile, "%s", buff); // first word is read, need to read 3 more words
                if (3 == sscanf(buff, "%d/%d/%d", &v, &t, &n))
                {
                    currentIndex->v = v - 1;
                    currentIndex->t = t - 1;
                    currentIndex->n = n - 1;
                    ++currentIndex;

                    fscanf(pFile, "%d/%d/%d", &v, &t, &n);
                    currentIndex->v = v - 1;
                    currentIndex->t = t - 1;
                    currentIndex->n = n - 1;
                    ++currentIndex;

                    fscanf(pFile, "%d/%d/%d", &v, &t, &n);
                    currentIndex->v = v - 1;
                    currentIndex->t = t - 1;
                    currentIndex->n = n - 1;
                    ++currentIndex;
                }
                /* TODO: add support for more index format
                 * 1. %d/%d
                 * 2. %d
                 * 3. %d/%d/%d
                 */
                break;
            }
            case 'v':
            {
                switch (buff[1])
                {
                    case '\0':
                    {
                        fgets(buff, sizeof(buff), pFile);
                        sscanf(buff, "%f %f %f", &currentPosition->x, &currentPosition->y, &currentPosition->z);
                        currentPosition++;
                        break;
                    }
                    case 'n':
                    {
                        fgets(buff, sizeof(buff), pFile);
                        sscanf(buff, "%f %f %f", &currentNormal->x, &currentNormal->y, &currentNormal->z);
                        currentNormal++;
                        break;
                    }
                    case 't':
                    {
                        fgets(buff, sizeof(buff), pFile);
                        sscanf(buff, "%f %f", &currentTexel->u, &currentTexel->v);
                        currentTexel++;
                        break;
                    }
                }
                break;
            }
            case '#':
            {
                // comment
            }
            default:
            {
                // read and dump content
                fgets(buff, sizeof(buff), pFile);
                break;
            }
        }
    }

    fclose(pFile);
    /* Convert input indices into output indices */
    uint32_t nInts = nPositions * nTexels * nNormals;
    int*     map   = (int*)malloc(sizeof(int) * nInts);
    for (uint32_t idx = 0U; idx < nInts; ++idx)
    {
        map[idx] = -1;
    }

    /*
     nOuterMatrix = nPositions
     nMiddleMatrix = nTexels
     nInnerMatrix = nNormals
    */

    pModel->pIndices = (uint32_t*)malloc(sizeof(uint32_t) * nInputIndices);
    // Vertex*
    uint32_t outIndex = 0;
    for (uint32_t idx = 0U; idx < nInputIndices; ++idx)
    {
        Index* pIndex = &pInputIndices[idx];
        int    val    = map[nNormals * nTexels * pIndex->v + nNormals * pIndex->t + pIndex->n];
        if (-1 == val)
        {
            val                                                                    = outIndex++;
            map[nNormals * nTexels * pIndex->v + nNormals * pIndex->t + pIndex->n] = val;
        }
        pModel->pIndices[idx] = val;
    }
    fprintf(stdout, "nOutIndices: %d\n", outIndex);
    pModel->pVertices = (Vertex*)malloc(sizeof(Vertex) * outIndex);
    for (uint32_t idx = 0U; idx < nInputIndices; ++idx)
    {
        Index* pIndex = &pInputIndices[idx];
        int    val    = map[nNormals * nTexels * pIndex->v + nNormals * pIndex->t + pIndex->n];
        if (-1 == val)
        {
            fprintf(stdout, "Something is wrong, this statement should not be printed\n");
        }
        else
        {
            pModel->pVertices[val].position = pPositions[pIndex->v];
            pModel->pVertices[val].texel    = pTexels[pIndex->t];
            pModel->pVertices[val].normal   = pNormals[pIndex->n];
        }
    }
    pModel->header.nIndices  = nInputIndices;
    pModel->header.nVertices = outIndex;

    free(map);
    free(pInputIndices);
    free(pPositions);
    free(pNormals);
    free(pTexels);
    return (0);
}

void unloadModel(struct Model* pModel)
{
    if (NULL != pModel->pIndices)
    {
        free(pModel->pIndices);
    }

    if (NULL != pModel->pVertices)
    {
        free(pModel->pVertices);
    }
}

int processMaterialFile(char* filename, struct Model* pModel)
{
    char             buff[128];
    int32_t          nMaterials = 0;
    struct Material* pMaterial  = NULL;
    struct Material* pMaterials = NULL;
    int32_t          idx        = 0U;
    FILE*            pFile      = NULL;

    pFile = fopen(filename, "r");
    if (NULL == pFile)
    {
        fprintf(stderr, "Failed to read material file \"%s\"\n", filename);
        return (-1);
    }

    while (EOF != fscanf(pFile, "%s", buff))
    {
        switch (buff[0])
        {
            case 'n': /* newmtl */
                fgets(buff, sizeof(buff), pFile);
                ++nMaterials;
                sscanf(buff, "%s %s", buff, buff);
                break;
            case '#':
            {
                // comment
                // line is read and content is discarded
            }
            default:
                /* eat up rest of line */
                fgets(buff, sizeof(buff), pFile);
                break;
        }
    }
    // pModel->nMaterials = nMaterials;
    pMaterials = (struct Material*)malloc(sizeof(struct Material) * nMaterials);

    /* reset to beginning of file */
    (void)fseek(pFile, 0L, SEEK_SET);

    /* set the default material */
    for (idx = 0; idx < nMaterials; idx++)
    {
        pMaterial               = pMaterials + idx;
        pMaterial->name         = NULL;
        pMaterial->shininess    = 65.0f;
        pMaterial->rDiffuse[0]  = 0.8f;
        pMaterial->rDiffuse[1]  = 0.8f;
        pMaterial->rDiffuse[2]  = 0.8f;
        pMaterial->rDiffuse[3]  = 1.0f;
        pMaterial->rAmbient[0]  = 0.2f;
        pMaterial->rAmbient[1]  = 0.2f;
        pMaterial->rAmbient[2]  = 0.2f;
        pMaterial->rAmbient[3]  = 1.0f;
        pMaterial->rSpecular[0] = 0.0f;
        pMaterial->rSpecular[1] = 0.0f;
        pMaterial->rSpecular[2] = 0.0f;
        pMaterial->rSpecular[3] = 1.0f;
        pMaterial->emmission[0] = 0.0f;
        pMaterial->emmission[1] = 0.0f;
        pMaterial->emmission[2] = 0.0f;
        pMaterial->emmission[3] = 1.0f;
    }
    idx = -1;

    while (EOF != fscanf(pFile, "%s", buff))
    {
        switch (buff[0])
        {
            case 'n': /* newmtl */
                ++idx;
                pMaterial = pMaterials + idx;
                fgets(buff, sizeof(buff), pFile);
                sscanf(buff, "%s %s", buff, buff);
                pMaterial->name = strdup(buff);
                break;
            case 'N':
            {
                if ('s' == buff[1])
                {
                    // shininess - focus of secular hightlight
                    fgets(buff, sizeof(buff), pFile);
                    sscanf(buff, "%f", &pMaterial->shininess);
                    pMaterial->shininess /= 1000.0f;
                    pMaterial->shininess *= 128.0f;
                }
                else if ('i' == buff[1])
                {
                    // optical density - refrative index
                    fgets(buff, sizeof(buff), pFile);
                    sscanf(buff, "%f", &pMaterial->opeticalDensity);
                }
                else
                {
                    fprintf(stderr, "Unknown command: \"%s\"\n", buff);
                }

                break;
            }
            case 'K':
            {
                switch (buff[1])
                {
                    case 'a':
                    {
                        fgets(buff, sizeof(buff), pFile);
                        sscanf(buff, "%f %f %f", pMaterial->rAmbient, pMaterial->rAmbient + 1, pMaterial->rAmbient + 2);
                        break;
                    }
                    case 's':
                    {
                        fgets(buff, sizeof(buff), pFile);
                        sscanf(buff, "%f %f %f", pMaterial->rSpecular, pMaterial->rSpecular + 1, pMaterial->rSpecular + 2);
                        break;
                    }
                    case 'd':
                    {
                        fgets(buff, sizeof(buff), pFile);
                        sscanf(buff, "%f %f %f", pMaterial->rDiffuse, pMaterial->rDiffuse + 1, pMaterial->rDiffuse + 2);
                        break;
                    }
                    case 'e':
                    {
                        fgets(buff, sizeof(buff), pFile);
                        sscanf(buff, "%f %f %f", pMaterial->emmission, pMaterial->emmission + 1, pMaterial->emmission + 2);
                        break;
                    }
                    default:
                    {
                        fprintf(stdout, "Unknown command \"%s\"\n", buff);
                        break;
                    }
                }
                break;
            }
            case 'd': // dissolve factor
            {
                fgets(buff, sizeof(buff), pFile);
                sscanf(buff, "%f", &pMaterial->dissolveFactor);
                break;
            }
            case 'i':
            {
                fgets(buff, sizeof(buff), pFile);
                sscanf(buff, "%u", &pMaterial->illuminationModel);
                break;
            }
            case '#':
            {
                // comment
                // line is read and content is discarded
            }
            default:
                /* eat up rest of line */
                fgets(buff, sizeof(buff), pFile);
                break;
        }
    }
    // pModel->pMaterials = pMaterials;
    fclose(pFile);
    return (0);
}
void printMaterial(struct Material* pMaterial)
{
    fprintf(stdout, "Name: %s\n", pMaterial->name);
    fprintf(stdout, "Shininess: %f\n", pMaterial->shininess);
    fprintf(stdout, "Refractive Index: %f\n", pMaterial->opeticalDensity);
    fprintf(stdout, "Dissolve factor: %f\n", pMaterial->dissolveFactor);
    fprintf(stdout, "Texture file: %s\n", pMaterial->texturePath);
    fprintf(stdout, "Illumination model: %u\n", pMaterial->illuminationModel);
    fprintf(stdout, "Ambient: [%.2f %.2f %.2f %.2f]\n", pMaterial->rAmbient[0], pMaterial->rAmbient[1], pMaterial->rAmbient[2], pMaterial->rAmbient[3]);
    fprintf(stdout, "Diffuse: [%.2f %.2f %.2f %.2f]\n", pMaterial->rDiffuse[0], pMaterial->rDiffuse[1], pMaterial->rDiffuse[2], pMaterial->rDiffuse[3]);
    fprintf(stdout, "Specular: [%.2f %.2f %.2f %.2f]\n", pMaterial->rSpecular[0], pMaterial->rSpecular[1], pMaterial->rSpecular[2], pMaterial->rSpecular[3]);
    fprintf(stdout, "Emission: [%.2f %.2f %.2f %.2f]\n", pMaterial->emmission[0], pMaterial->emmission[1], pMaterial->emmission[2], pMaterial->emmission[3]);
    fprintf(stdout, "\n");
}

// static int findMaterial(struct Model* pModel, char* name)
// {
//     if (NULL != pModel->pMaterials)
//     {
//         struct Material* pMaterial = NULL;
//         for (uint32_t idx = 0U; idx < pModel->nMaterials; ++idx)
//         {
//             pMaterial = pModel->pMaterials + idx;
//             if (0 == strcmp(pMaterial->name, name))
//             {
//                 return idx;
//             }
//         }
//         fprintf(stderr, "Materials not loaded\n");
//     }
//     fprintf(stderr, "failed to find material \"%s\"\n", name);
//     return (-1);
// }

void deleteMaterials(struct Material* pMaterials, int nMaterials)
{
    if (NULL == pMaterials)
    {
        return;
    }
    struct Material* pMaterial;
    for (int32_t idx = 0U; idx < nMaterials; ++idx)
    {
        pMaterial = pMaterials + idx;
        if (NULL != pMaterial->name)
        {
            free(pMaterial->name);
        }
        if (NULL != pMaterial->texturePath)
        {
            free(pMaterial->texturePath);
        }
    }
    free(pMaterials);
    pMaterials = NULL;
}

int exportModel(Model* pModel, const char* pFileName)
{
    int res = -1;
    if (NULL == pModel || NULL == pModel->pIndices || NULL == pModel->pVertices)
    {
        fprintf(stderr, "Invalid model data, cannot export \n");
        res = -1;
    }
    else
    {
        FILE* pFile = fopen(pFileName, "wb");
        if (NULL == pFile)
        {
            fprintf(stderr, "Failed to open model file %s\n", pFileName);
            res = -1;
        }
        else
        {
            if (1 != fwrite(&pModel->header, sizeof(Header), 1, pFile))
            {
                fprintf(stderr, "Failed to write header\n");
                res = -1;
            }
            else
            {
                long totalWritten = 0;
                long val;
                do
                {
                    val = fwrite(pModel->pIndices + totalWritten, sizeof(uint32_t), pModel->header.nIndices - totalWritten, pFile);
                    if (val <= 0)
                    {
                        fprintf(stderr, "Something wrong happened\n");
                        fprintf(stderr, "Failed to write indices %ld\n", val);
                        res = -1;
                        break;
                    }
                    totalWritten += val;
                    fprintf(stderr, "write indices %ld/%ld\n", val, totalWritten);
                } while (totalWritten < pModel->header.nIndices);

                totalWritten = 0;
                do
                {
                    val = fwrite(pModel->pVertices + totalWritten, sizeof(Vertex), pModel->header.nVertices - totalWritten, pFile);
                    if (val <= 0)
                    {
                        fprintf(stderr, "Something wrong happened\n");
                        fprintf(stderr, "Failed to write vertices %ld\n", val);
                        res = -1;
                        break;
                    }
                    totalWritten += val;
                    fprintf(stderr, "write vertices %ld/%ld\n", val, totalWritten);
                } while (totalWritten < pModel->header.nVertices);
            }
            fclose(pFile);
        }
    }

    return res;
}

int loadModel(Model* pModel, const char* pFileName)
{
    int res = -1;
    if (NULL == pModel)
    {
        fprintf(stderr, "NULL model data, cannot load \n");
        res = -1;
    }
    else
    {
        FILE* pFile = fopen(pFileName, "rb");
        if (NULL == pFile)
        {
            fprintf(stderr, "Failed to open model file %s\n", pFileName);
            res = -1;
        }
        else
        {
            if (1 != fread(&pModel->header, sizeof(Header), 1, pFile))
            {
                fprintf(stderr, "Failed to read header\n");
                res = -1;
            }
            else
            {
                long totalRead = 0;
                pModel->pIndices  = (uint32_t*)malloc(sizeof(uint32_t) * pModel->header.nIndices);
                do
                {
                    long val = fread(pModel->pIndices + totalRead, sizeof(uint32_t), pModel->header.nIndices - totalRead, pFile);
                    if (0 >= val)
                    {
                        fprintf(stderr, "Failed to read indices\n");
                        res = -1;
                    }
                    totalRead += val;
                } while (pModel->header.nIndices > totalRead);

                totalRead         = 0;
                pModel->pVertices = (Vertex*)malloc(sizeof(Vertex) * pModel->header.nVertices);
                do
                {
                    long val = fread(pModel->pVertices + totalRead, sizeof(Vertex), pModel->header.nVertices - totalRead, pFile);
                    if (0 >= val)
                    {
                        fprintf(stderr, "Failed to read vertices\n");
                        res = -1;
                    }
                    totalRead += val;
                } while (pModel->header.nVertices > totalRead);

                fprintf(stdout, "%s: Model loaded successfully\n", __func__);
                res = 0;
            }
            fclose(pFile);
        }
    }

    return res;
}

//----
