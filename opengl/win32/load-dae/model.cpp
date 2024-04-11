#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include "load-model.h"
#include "OGL.h"

static int readBody(Model_t* pModel, FILE* pFile)
{
    pModel->body.pVertices = (Vertex_t *)malloc(sizeof(uint32_t)*pModel->header.nIndices + sizeof(Vertex_t)*pModel->header.nVertices);
    pModel->body.pIndices = (uint32_t *)(pModel->body.pVertices + pModel->header.nVertices);

    fread(pModel->body.pVertices, sizeof(Vertex_t), pModel->header.nVertices, pFile);
    fread(pModel->body.pIndices, sizeof(uint32_t), pModel->header.nIndices, pFile);
    return 0;
}

Model_t* loadModel(const char* pFilename)
{
    FILE* pFile = fopen(pFilename, "rb");
    Model_t *pModel = static_cast<Model_t*>(malloc(sizeof(Model_t)));
    if(NULL == pFile)
    {
        fprintf(gpFILE, "Failed to open file %s (errno = %d)", pFilename, errno);
    }
    else
    {
        fprintf(gpFILE, "File opened successfuly %s\n", pFilename);
        fread(&pModel->header, sizeof(Header_t), 1, pFile);
        readBody(pModel, pFile);
        fclose(pFile);
        fprintf(gpFILE, "File closed successfuly %s\n", pFilename);
        pFile = NULL;
    }
    return pModel;
}

static int freeBody(Body_t* pBody)
{
    if(NULL != pBody->pVertices)
    {
        free(pBody->pVertices);
    }

    return 0;
}

int unloadModel(Model_t* pModel)
{
    freeBody(&pModel->body);

    if(NULL != pModel)
    {
        free(pModel);
    }
    return 0;
}


// int main()
// {
//     Model_t* pModel = loadModel("float_data.bin");
    
//         printf("nPositions: %u\n", pModel->header.nVertices);
//         printf("nTexCoords: %u\n", pModel->header.nIndices);
//         for (uint32_t i = 0; i < pModel->header.nIndices; i++)
//         {
//             printf("%-04d", pModel->body.pIndices[i]);
//         }
//             printf("\n\n");
//         for (uint32_t i = 0; i < pModel->header.nVertices; i++)
//         {
//             for (uint32_t j = 0; j < 7; j++)
//             {
//                 printf("%7.3f", ((float *)(pModel->body.pVertices))[i* 7 + j]);
//             }
//             printf("\n");
            
//         }
        
//         printf("\n");
        
//     unloadModel(pModel);
// }