//
// Created by rohit on 14-04-2024.
//

#include <stdio.h>
#include "load.h"
#include "android/log.h"
#include "android/asset_manager.h"
#include <android/asset_manager_jni.h>
#include <jni.h>
#include <errno.h>
#include <dirent.h>

#define TAG "load.CPP"
extern FILE* gpFile;

int loadModel(Model* pModel, const char* pFileName)
{
    int res = -1;
    if (NULL == pModel)
    {
        fprintf(gpFile, "NULL model data, cannot load \n");
        res = -1;
    }
    else
    {
        FILE* pFile = fopen(pFileName, "rb");
        if (NULL == pFile)
        {
            fprintf(gpFile, "Failed to open model file %s (errno %d)\n", pFileName, errno);
            res = -1;
        }
        else
        {
            if (1 != fread(&pModel->header, sizeof(Header), 1, pFile))
            {
                fprintf(gpFile, "Failed to read header\n");
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
                        fprintf(gpFile, "Failed to read indices\n");
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
                        fprintf(gpFile, "Failed to read vertices\n");
                        res = -1;
                    }
                    totalRead += val;
                } while (pModel->header.nVertices > totalRead);

                fprintf(gpFile, "%s: Model loaded successfully\n\tName: %s\n\tnVertices: %d\n\tnIndices: %d\n", __func__, pModel->header.name, pModel->header.nVertices, pModel->header.nIndices);
                res = 0;
            }
            fclose(pFile);
        }
    }

    return res;
}

void unloadModel(struct Model *pModel) {

    fprintf(gpFile, "Unloading model\n");
    if (NULL != pModel->pIndices) {
        free(pModel->pIndices);
    }

    if (NULL != pModel->pVertices) {
        free(pModel->pVertices);
    }
}
